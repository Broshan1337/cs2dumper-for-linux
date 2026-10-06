//! Class vtable discovery and dumping.
//!
//! Scans each target module's mapped image for Itanium-C++ vtable structures:
//! an `offset-to-top` i64 (small, usually 0 or negative), followed by a
//! pointer to the class's `std::type_info` object, followed by function
//! pointers into the module's own code. Because the module is read from the
//! live process, every pointer is already relocated — no ELF relocation
//! bookkeeping is needed (the offline-file variant of this scan has to
//! maintain a RELA addend map; the live one does not).
//!
//! The class name comes from the typeinfo object's embedded (mangled) name,
//! so the output is keyed by demangled class name exactly like the schema
//! dump, which is what makes per-update slot-drift diffs trivial.

use std::collections::BTreeMap;

use anyhow::{Result, anyhow};
use log::{debug, info, warn};

use memflow::prelude::v1::*;

/// class name -> dump of its primary vtable (offset-to-top == 0).
#[derive(Debug, Clone, serde::Serialize)]
pub struct Vtable {
    /// Offset-to-top of the primary table (0 for the complete object).
    pub offset_to_top: i64,
    /// RVA of the vtable's first function slot.
    pub primary: u64,
    /// Number of secondary (base-class subobject) tables sharing this typeinfo.
    pub secondary_tables: usize,
    /// Function RVAs per slot.
    pub slots: Vec<u64>,
}

pub type VtableMap = BTreeMap<String, BTreeMap<String, Vtable>>;

/// Modules to scan. Everything the rest of the dump touches plus the other
/// game modules that carry game classes (rendering, animation, netcode).
const MODULES: [&str; 12] = [
    "libclient.so",
    "libserver.so",
    "libengine2.so",
    "libpanorama.so",
    "libschemasystem.so",
    "libnetworksystem.so",
    "libscenesystem.so",
    "libparticles.so",
    "libsoundsystem.so",
    "libanimationsystem.so",
    "libmaterialsystem2.so",
    "libfilesystem_stdio.so",
];

/// Cap on slots collected from one table (some schema-heavy interfaces have
/// hundreds of entries; this is generous by an order of magnitude).
const MAX_SLOTS: usize = 4096;

/// How many consecutive non-code qwords end a table. The tail of a vtable can
/// contain weak/zero entries (the 2026-09-24 VoiceTap lesson: a vtable with a
/// NULL slot 0 exists in the wild), so this is NULL-tolerant instead of
/// stopping at the first non-code entry.
const TRAILING_MISS_LIMIT: usize = 3;

pub fn vtables<P: Process + MemoryView>(process: &mut P) -> Result<VtableMap> {
    let mut map = VtableMap::new();

    for module_name in MODULES {
        let module = match crate::analysis::module_by_name_retry(process, module_name) {
            Ok(module) => module,
            Err(err) => {
                warn!("skipping vtables for {}: {}", module_name, err);
                continue;
            }
        };

        let image = process
            .read_raw(module.base, module.size as _)
            .data_part()?;
        let base = module.base.to_umem();
        let len = image.len() as u64;

        let tables = scan_vtables(&image, base, len);

        let mut out = BTreeMap::new();
        for (name, (primary, others)) in tables {
            let (rva, ott, slots) = primary;
            out.insert(
                name,
                Vtable {
                    offset_to_top: ott,
                    primary: rva,
                    secondary_tables: others,
                    slots,
                },
            );
        }

        info!(
            "found {} class vtables in {}",
            out.len(),
            module_name
        );
        map.insert(module_name.to_string(), out);
    }

    if map.is_empty() {
        return Err(anyhow!("no vtables found in any module"));
    }

    Ok(map)
}

/// typeinfo-name validation: either `<digits><Identifier>` chunks (Itanium)
/// or a `N...E` nested name, optionally with `I...E` template args. Anything
/// else is rejected as a false-positive anchor.
fn looks_like_mangled(name: &[u8]) -> Option<String> {
    if name.is_empty() || name.len() > 512 {
        return None;
    }
    let s = std::str::from_utf8(name).ok()?;
    let b = s.as_bytes();
    let valid_char =
        |c: u8| c.is_ascii_alphanumeric() || c == b'_' || c == b'$' || c == b'<' || c == b'>';

    if b[0] == b'N' {
        // nested name: N [CV-quals] <len><id>... [I <len><id>... E] E
        let mut i = 1;
        while i < b.len() && (b[i] == b'V' || b[i] == b'K' || b[i] == b'r') {
            i += 1;
        }
        let mut depth = 1usize;
        let mut saw_component = false;
        while i < b.len() {
            match b[i] {
                b'E' => {
                    depth -= 1;
                    i += 1;
                    if depth == 0 {
                        if i != b.len() {
                            return None;
                        }
                        return Some(s.to_string());
                    }
                }
                b'I' => {
                    // template args: skip to matching E at same depth
                    depth += 1;
                    i += 1;
                }
                c if c.is_ascii_digit() => {
                    let mut len = 0usize;
                    while i < b.len() && b[i].is_ascii_digit() {
                        len = len * 10 + (b[i] - b'0') as usize;
                        i += 1;
                    }
                    if len == 0 || i + len > b.len() {
                        return None;
                    }
                    if !b[i..i + len]
                        .iter()
                        .all(|c| valid_char(*c))
                    {
                        return None;
                    }
                    i += len;
                    saw_component = true;
                }
                _ => return None,
            }
        }
        let _ = saw_component;
        None
    } else {
        // flat: <len><id>[<len><id>...]
        let mut i = 0usize;
        while i < b.len() {
            if !b[i].is_ascii_digit() {
                return None;
            }
            let mut len = 0usize;
            while i < b.len() && b[i].is_ascii_digit() {
                len = len * 10 + (b[i] - b'0') as usize;
                i += 1;
            }
            if len == 0 || i + len > b.len() {
                return None;
            }
            if !b[i..i + len].iter().all(|c| valid_char(*c)) {
                return None;
            }
            i += len;
        }
        Some(s.to_string())
    }
}

/// Demangle the subset we care about into `ns::Class` / `Class` / `ns::Outer::Inner`.
pub fn demangle(mangled: &str) -> String {
    let b = mangled.as_bytes();
    let mut out = String::new();
    let mut parts: Vec<String> = Vec::new();

    let read_component = |i: &mut usize| -> Option<String> {
        if *i >= b.len() {
            return None;
        }
        let start = *i;
        while *i < b.len() && b[*i].is_ascii_digit() {
            *i += 1;
        }
        if *i == start {
            return None;
        }
        let len: usize = mangled[start..*i].parse().ok()?;
        let end = *i + len;
        if end > b.len() {
            return None;
        }
        let s = mangled[*i..end].to_string();
        *i = end;
        Some(s)
    };

    let mut i = 0usize;
    if b[0] == b'N' {
        i = 1;
        while i < b.len() && (b[i] == b'V' || b[i] == b'K' || b[i] == b'r') {
            i += 1;
        }
        let depth = 1usize;
        while i < b.len() {
            match b[i] {
                b'E' => break,
                b'I' => {
                    // template args: keep raw
                    let mut d = 1usize;
                    let start = i;
                    i += 1;
                    while i < b.len() && d > 0 {
                        match b[i] {
                            b'I' => d += 1,
                            b'E' => d -= 1,
                            _ => {}
                        }
                        i += 1;
                    }
                    if let Some(last) = parts.last_mut() {
                        last.push('<');
                        last.push_str(&mangled[start + 1..i.saturating_sub(1)]);
                        last.push('>');
                    }
                }
                _ => {
                    if let Some(s) = read_component(&mut i) {
                        parts.push(s);
                    } else {
                        return mangled.to_string();
                    }
                }
            }
            let _ = depth;
        }
    } else {
        while i < b.len() {
            match read_component(&mut i) {
                Some(s) => parts.push(s),
                None => return mangled.to_string(),
            }
        }
    }

    if parts.is_empty() {
        return mangled.to_string();
    }
    out.push_str(&parts.join("::"));
    out
}

/// Candidate check for a vtable header at byte offset `off`:
/// [offset-to-top i64 in range][typeinfo ptr into module], and the typeinfo's
/// embedded name must validate as a mangled name. The name check is what
/// keeps an interior NULL slot followed by a code pointer from being mistaken
/// for the next table's header (the 2026-09-24 VoiceTap NULL-slot lesson).
fn is_vtable_header(image: &[u8], base: u64, len: u64, off: usize) -> bool {
    if off + 16 > image.len() {
        return false;
    }
    let ott = i64::from_le_bytes(image[off..off + 8].try_into().unwrap());
    if !(-0x1_0000i64..=0i64).contains(&ott) {
        return false;
    }
    let ti = u64::from_le_bytes(image[off + 8..off + 16].try_into().unwrap());
    if ti < base + 0x1000 || ti >= base + len {
        return false;
    }
    let tio = (ti - base) as usize;
    if tio + 16 > image.len() {
        return false;
    }
    let name_ptr = u64::from_le_bytes(image[tio + 8..tio + 16].try_into().unwrap());
    if name_ptr < base || name_ptr >= base + len {
        return false;
    }
    let no = (name_ptr - base) as usize;
    let name_end = image[no..(no + 512).min(image.len())]
        .iter()
        .position(|b| *b == 0);
    let Some(end) = name_end else {
        return false;
    };
    looks_like_mangled(&image[no..no + end]).is_some()
}

/// locate the zero-terminated name string for the typeinfo at `ti` (module bytes)
fn typeinfo_name<'a>(image: &'a [u8], base: u64, len: u64, ti: u64) -> Option<&'a [u8]> {
    let tio = (ti - base) as usize;
    if tio + 16 > image.len() {
        return None;
    }
    let name_ptr = u64::from_le_bytes(image[tio + 8..tio + 16].try_into().unwrap());
    if name_ptr < base || name_ptr >= base + len {
        return None;
    }
    let no = (name_ptr - base) as usize;
    let end = image[no..(no + 512).min(image.len())]
        .iter()
        .position(|b| *b == 0)
        .map(|pos| no + pos)?;
    Some(&image[no..end])
}

type Table = (u64, i64, Vec<u64>); // (primary rva, ott, slots)

fn scan_vtables(image: &[u8], base: u64, len: u64) -> BTreeMap<String, (Table, usize)> {
    let mut classes: BTreeMap<String, (Table, usize)> = BTreeMap::new();

    let mut p = 0usize;
    while p + 32 <= image.len() {
        let ott = i64::from_le_bytes(image[p..p + 8].try_into().unwrap());
        if !(-0x1_0000i64..=0i64).contains(&ott) {
            p += 8;
            continue;
        }
        let ti = u64::from_le_bytes(image[p + 8..p + 16].try_into().unwrap());
        if ti < base + 0x1000 || ti >= base + len {
            p += 8;
            continue;
        }

        // typeinfo object: { typeinfo vptr, name pointer }
        let Some(name_bytes) = typeinfo_name(image, base, len, ti) else {
            p += 8;
            continue;
        };
        let mangled = match looks_like_mangled(name_bytes) {
            Some(s) => s,
            None => {
                p += 8;
                continue;
            }
        };
        let name = demangle(&mangled);

        // collect slots; the table ends at the NEXT vtable header (a validated
        // [offset-to-top, typeinfo] pair) or after TRAILING_MISS_LIMIT
        // consecutive non-code qwords (weak/NULL entries exist in the wild).
        let mut slots = Vec::with_capacity(64);
        let mut misses = 0usize;
        let mut q = (p + 16) as usize;
        while q + 8 <= image.len() && slots.len() < MAX_SLOTS {
            if is_vtable_header(image, base, len, q) {
                break;
            }
            let fnptr = u64::from_le_bytes(image[q..q + 8].try_into().unwrap());
            if fnptr >= base && fnptr < base + len && fnptr & 0xF == 0 {
                slots.push(fnptr - base);
                misses = 0;
            } else {
                misses += 1;
                if misses >= TRAILING_MISS_LIMIT {
                    break;
                }
            }
            q += 8;
        }

        if slots.is_empty() {
            p += 8;
            continue;
        }

        let entry: Table = (p as u64 + 16, ott, slots);
        match classes.get_mut(&name) {
            Some((primary, secondary)) => {
                // keep the ott==0 (complete-object) table as primary
                if ott == 0 && primary.1 != 0 {
                    *secondary += 1;
                    let prev = primary.clone();
                    *primary = entry;
                    let _ = prev;
                } else {
                    *secondary += 1;
                }
            }
            None => {
                classes.insert(name, (entry, 0));
            }
        }

        // continue scanning AFTER the collected table (entries are part of it)
        p = q;
    }

    classes
}

/// Debug helper used during development.
#[allow(dead_code)]
fn debug_count(map: &VtableMap) {
    for (module, classes) in map {
        debug!("{}: {} classes", module, classes.len());
    }
}
