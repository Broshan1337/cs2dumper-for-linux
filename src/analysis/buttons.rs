use std::collections::BTreeMap;

use anyhow::{Result, anyhow};

use log::debug;

use memflow::prelude::v1::*;

use skidscan_macros::signature;

use crate::memory::address;
use crate::source2::KeyButton;

pub type ButtonMap = BTreeMap<String, umem>;

pub fn buttons<P: Process + MemoryView>(process: &mut P) -> Result<ButtonMap> {
    let module = process.module_by_name("libclient.so")?;

    let buf = process
        .read_raw(module.base, module.size as _)
        .data_part()?;

    // Anchored on the KeyButton registration helper, which links each button into
    // the global list: `head = this; this->next = old_head`. The trailing
    // `mov [rbx+0x88], rax` writes KeyButton::next and makes the match unique.
    let list_addr = signature!("48 8B 05 ? ? ? ? 48 89 1D ? ? ? ? 48 89 83 88 00 00 00")
        .scan(&buf)
        .and_then(|result| address::resolve_rip(process, module.base + result).ok())
        .ok_or_else(|| anyhow!("unable to read button list address"))?;

    read_buttons(process, &module, list_addr)
}

fn read_buttons(
    mem: &mut impl MemoryView,
    module: &ModuleInfo,
    list_addr: Address,
) -> Result<ButtonMap> {
    let mut result = ButtonMap::new();

    let mut button_ptr = Pointer64::<KeyButton>::from(mem.read_addr64(list_addr).data_part()?);

    while !button_ptr.is_null() {
        let button = mem.read_ptr(button_ptr).data_part()?;
        let name = mem.read_utf8_lossy(button.name.address(), 32).data_part()?;

        let state_addr = button_ptr.address() + offset_of!(KeyButton.state);

        if let Some(state_rva) = state_addr.to_umem().checked_sub(module.base.to_umem()) {
            debug!(
                "found \"{}\" at {:#X} ({} + {:#X})",
                name,
                state_addr.to_umem(),
                module.name,
                state_rva
            );

            result.insert(name, state_rva);
        }

        button_ptr = button.next;
    }

    Ok(result)
}
