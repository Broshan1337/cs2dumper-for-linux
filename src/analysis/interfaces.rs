use std::collections::BTreeMap;

use anyhow::{Result, anyhow};

use log::debug;

use memflow::prelude::v1::*;

use skidscan_macros::signature;

use crate::memory::{address, elf};
use crate::source2::InterfaceReg;

pub type InterfaceMap = BTreeMap<String, BTreeMap<String, umem>>;

/// Resolves the head of a module's `s_pInterfaceRegs` linked list.
///
/// The list pointer is loaded in the prologue of the exported `CreateInterface`
/// function as `mov rbx, [rip + disp32]`. Anchoring the scan to that export keeps the
/// match unambiguous -- the same byte sequence occurs elsewhere in larger modules such
/// as libclient.so.
pub fn interface_list_head<P: Process + MemoryView>(
    process: &mut P,
    module: &ModuleInfo,
    buf: &[u8],
) -> Result<Address> {
    let export = elf::export_rva(buf, module.base.to_umem(), "CreateInterface")
        .ok_or_else(|| anyhow!("no CreateInterface export in {}", module.name))?;

    let start = export as usize;
    let end = (start + 0x40).min(buf.len());

    let prologue = buf
        .get(start..end)
        .ok_or_else(|| anyhow!("CreateInterface out of bounds in {}", module.name))?;

    let offset = signature!("48 8B 1D ? ? ? ? 48 85 DB")
        .scan(prologue)
        .ok_or_else(|| anyhow!("outdated interface list pattern in {}", module.name))?;

    let list_ptr = address::resolve_rip(process, module.base + start + offset)?;

    process.read_addr64(list_ptr).data_part().map_err(Into::into)
}

pub fn interfaces<P: Process + MemoryView>(process: &mut P) -> Result<InterfaceMap> {
    process
        .module_list()?
        .iter()
        .filter_map(|module| {
            let buf = process
                .read_raw(module.base, module.size as _)
                .data_part()
                .ok()?;

            let list_head = interface_list_head(process, module, &buf).ok()?;

            read_interfaces(process, module, list_head)
                .ok()
                .filter(|ifaces| !ifaces.is_empty())
                .map(|ifaces| Ok((module.name.to_string(), ifaces)))
        })
        .collect()
}

fn read_interfaces(
    mem: &mut impl MemoryView,
    module: &ModuleInfo,
    list_head: Address,
) -> Result<BTreeMap<String, umem>> {
    let mut result = BTreeMap::new();

    let mut reg_ptr = Pointer64::<InterfaceReg>::from(list_head);

    while !reg_ptr.is_null() {
        let reg = mem.read_ptr(reg_ptr).data_part()?;
        let name = mem.read_utf8_lossy(reg.name.address(), 128).data_part()?;

        let instance_addr = reg.create_fn.address();

        if let Some(instance_rva) = instance_addr.to_umem().checked_sub(module.base.to_umem()) {
            debug!(
                "found \"{}\" at {:#X} ({} + {:#X})",
                name,
                instance_addr.to_umem(),
                module.name,
                instance_rva
            );

            result.insert(name, instance_rva);
        }

        reg_ptr = reg.next;
    }

    Ok(result)
}
