//! Minimal ELF64 dynamic-symbol lookup over a module image read from process memory.
//!
//! memflow's `Process::module_export_by_name` cannot be used for this. Its ELF path
//! locates `PT_DYNAMIC` by file offset (`p_offset`) but resolves `DT_SYMTAB` and
//! `DT_STRTAB` by virtual address. Those only agree when a segment's file and virtual
//! offsets match, which is not the case for the Source 2 modules -- libclient.so maps
//! file `0x42AD320` to vaddr `0x42AF320` -- so the lookup silently finds nothing.
//!
//! Everything here indexes the image by virtual address, which is what a module read
//! out of process memory actually looks like.

use memflow::prelude::v1::*;

const PT_DYNAMIC: u32 = 2;

const DT_NULL: i64 = 0;
const DT_STRTAB: i64 = 5;
const DT_SYMTAB: i64 = 6;
const DT_SYMENT: i64 = 11;

const SYM_SIZE: usize = 24;

#[inline]
fn read_u16(buf: &[u8], off: usize) -> Option<u16> {
    buf.get(off..off + 2)
        .and_then(|b| b.try_into().ok())
        .map(u16::from_le_bytes)
}

#[inline]
fn read_u32(buf: &[u8], off: usize) -> Option<u32> {
    buf.get(off..off + 4)
        .and_then(|b| b.try_into().ok())
        .map(u32::from_le_bytes)
}

#[inline]
fn read_u64(buf: &[u8], off: usize) -> Option<u64> {
    buf.get(off..off + 8)
        .and_then(|b| b.try_into().ok())
        .map(u64::from_le_bytes)
}

fn c_str(buf: &[u8], off: usize) -> Option<&str> {
    let rest = buf.get(off..)?;
    let end = rest.iter().position(|&c| c == 0)?;

    std::str::from_utf8(&rest[..end]).ok()
}

/// Returns the module-relative address of an exported symbol, or `None` if the image
/// is not ELF64 or does not export it.
///
/// `base` is the module's load address. It is needed because glibc relocates the
/// `.dynamic` section in place: in a live image `DT_SYMTAB` / `DT_STRTAB` hold
/// absolute addresses, while on disk they are module-relative. Both are accepted.
pub fn export_rva(buf: &[u8], base: umem, name: &str) -> Option<umem> {
    if !buf.starts_with(b"\x7fELF") || buf.get(4).copied() != Some(2) {
        return None;
    }

    let e_phoff = read_u64(buf, 0x20)? as usize;
    let e_phentsize = read_u16(buf, 0x36)? as usize;
    let e_phnum = read_u16(buf, 0x38)? as usize;

    let mut dyn_addr = 0;
    let mut dyn_size = 0;

    for i in 0..e_phnum {
        let phdr = e_phoff + i * e_phentsize;

        if read_u32(buf, phdr)? == PT_DYNAMIC {
            dyn_addr = read_u64(buf, phdr + 0x10)? as usize;
            dyn_size = read_u64(buf, phdr + 0x20)? as usize;

            break;
        }
    }

    if dyn_size == 0 {
        return None;
    }

    let mut symtab = None;
    let mut strtab = None;
    let mut syment = SYM_SIZE;

    let mut entry = dyn_addr;

    while entry + 16 <= dyn_addr + dyn_size {
        let tag = read_u64(buf, entry)? as i64;
        let val = read_u64(buf, entry + 8)?;

        entry += 16;

        match tag {
            DT_NULL => break,
            DT_STRTAB => strtab = Some(val as usize),
            DT_SYMTAB => symtab = Some(val as usize),
            DT_SYMENT => syment = val as usize,
            _ => {}
        }
    }

    // Normalise the loader's absolute pointers back to module-relative offsets.
    let rebase = |v: usize| -> usize {
        if base != 0 && v as umem >= base {
            (v as umem - base) as usize
        } else {
            v
        }
    };

    let (symtab, strtab) = (rebase(symtab?), rebase(strtab?));

    if syment == 0 || strtab <= symtab {
        return None;
    }

    // There is no explicit symbol count in the dynamic section; the table runs up to
    // the start of the string table that immediately follows it.
    let count = (strtab - symtab) / syment;

    (0..count).find_map(|i| {
        let sym = symtab + i * syment;

        let st_name = read_u32(buf, sym)? as usize;
        let st_shndx = read_u16(buf, sym + 6)?;
        let st_value = read_u64(buf, sym + 8)?;

        // SHN_UNDEF entries are imports, not exports.
        if st_name == 0 || st_shndx == 0 {
            return None;
        }

        (c_str(buf, strtab + st_name)? == name).then_some(st_value as umem)
    })
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Lays each `PT_LOAD` at its virtual address, reproducing how a module looks once
    /// mapped into process memory (which is what `export_rva` expects).
    fn memory_image(data: &[u8]) -> Vec<u8> {
        let e_phoff = read_u64(data, 0x20).unwrap() as usize;
        let e_phentsize = read_u16(data, 0x36).unwrap() as usize;
        let e_phnum = read_u16(data, 0x38).unwrap() as usize;

        let mut loads = Vec::new();

        for i in 0..e_phnum {
            let phdr = e_phoff + i * e_phentsize;

            if read_u32(data, phdr).unwrap() == 1 {
                let p_offset = read_u64(data, phdr + 0x08).unwrap() as usize;
                let p_vaddr = read_u64(data, phdr + 0x10).unwrap() as usize;
                let p_filesz = read_u64(data, phdr + 0x20).unwrap() as usize;
                let p_memsz = read_u64(data, phdr + 0x28).unwrap() as usize;

                loads.push((p_offset, p_vaddr, p_filesz, p_memsz));
            }
        }

        let end = loads.iter().map(|l| l.1 + l.3).max().unwrap();

        let mut image = vec![0u8; end];

        for (off, vaddr, filesz, _) in loads {
            image[vaddr..vaddr + filesz].copy_from_slice(&data[off..off + filesz]);
        }

        image
    }

    /// Rewrites `DT_SYMTAB` / `DT_STRTAB` to absolute addresses the way glibc's loader
    /// does when it maps a shared object, so the test exercises a realistic live image.
    fn relocate_dynamic(image: &mut [u8], base: u64) {
        let e_phoff = read_u64(image, 0x20).unwrap() as usize;
        let e_phentsize = read_u16(image, 0x36).unwrap() as usize;
        let e_phnum = read_u16(image, 0x38).unwrap() as usize;

        for i in 0..e_phnum {
            let phdr = e_phoff + i * e_phentsize;

            if read_u32(image, phdr).unwrap() != PT_DYNAMIC {
                continue;
            }

            let dyn_addr = read_u64(image, phdr + 0x10).unwrap() as usize;
            let dyn_size = read_u64(image, phdr + 0x20).unwrap() as usize;

            let mut entry = dyn_addr;

            while entry + 16 <= dyn_addr + dyn_size {
                let tag = read_u64(image, entry).unwrap() as i64;

                if tag == DT_NULL {
                    break;
                }

                if matches!(tag, DT_STRTAB | DT_SYMTAB) {
                    let val = read_u64(image, entry + 8).unwrap();

                    image[entry + 8..entry + 16].copy_from_slice(&(val + base).to_le_bytes());
                }

                entry += 16;
            }

            return;
        }
    }

    /// Set `CS2_DUMPER_ELF_TEST` to a Source 2 `.so` to check that `CreateInterface`
    /// resolves out of a memory image. Skipped when unset.
    #[test]
    fn resolves_create_interface() {
        let Ok(path) = std::env::var("CS2_DUMPER_ELF_TEST") else {
            eprintln!("CS2_DUMPER_ELF_TEST not set, skipping");
            return;
        };

        let data = std::fs::read(&path).expect("unable to read module");
        let image = memory_image(&data);

        // As read straight off disk, DT_ pointers are still module-relative.
        let rva = export_rva(&image, 0, "CreateInterface")
            .unwrap_or_else(|| panic!("CreateInterface not found in {}", path));

        eprintln!("{}: CreateInterface at {:#X}", path, rva);

        assert_ne!(rva, 0);

        // And again with the loader's relocation applied, which is what the dumper
        // actually sees when it reads the module out of a running process.
        const BASE: u64 = 0x7F0000000000;

        let mut relocated = image.clone();

        relocate_dynamic(&mut relocated, BASE);

        assert_eq!(
            export_rva(&relocated, BASE, "CreateInterface"),
            Some(rva),
            "relocated .dynamic must resolve to the same RVA"
        );
    }
}
