pub use buttons::*;
pub use interfaces::*;
pub use offsets::*;
pub use schemas::*;

use std::any::type_name;
use std::thread;
use std::time::Duration;

use anyhow::{Result, anyhow};

use log::{error, info, warn};

use memflow::prelude::v1::*;

mod buttons;
mod interfaces;
mod offsets;
mod schemas;

/// Number of attempts made when looking up a module.
///
/// memflow-native re-reads `/proc/<pid>/maps` for every entry of the module
/// list. The game changes its memory map constantly (transient mmaps, worker
/// thread stacks), so a lookup can transiently fail with `module not found`
/// when the map list shifts between two reads. Retrying makes the pass
/// deterministic.
const MODULE_LOOKUP_ATTEMPTS: usize = 8;

const MODULE_LOOKUP_DELAY: Duration = Duration::from_millis(50);

pub fn module_by_name_retry<P: Process + MemoryView>(
    process: &mut P,
    name: &str,
) -> Result<ModuleInfo> {
    let mut last_err = None;

    for _ in 0..MODULE_LOOKUP_ATTEMPTS {
        match process.module_by_name(name) {
            Ok(module) => return Ok(module),
            Err(err) => {
                last_err = Some(err);

                thread::sleep(MODULE_LOOKUP_DELAY);
            }
        }
    }

    Err(last_err.unwrap().into())
}

pub fn module_list_retry<P: Process + MemoryView>(process: &mut P) -> Result<Vec<ModuleInfo>> {
    let mut last_err = None;

    for _ in 0..MODULE_LOOKUP_ATTEMPTS {
        match process.module_list() {
            Ok(modules) if !modules.is_empty() => return Ok(modules),
            Err(err) => last_err = Some(err),
            Ok(_) => last_err = None,
        }

        thread::sleep(MODULE_LOOKUP_DELAY);
    }

    Err(last_err.map(Into::into).unwrap_or_else(|| anyhow!("module list is empty")))
}

#[derive(Debug)]
pub struct AnalysisResult {
    pub buttons: ButtonMap,
    pub interfaces: InterfaceMap,
    pub offsets: OffsetMap,
    pub schemas: SchemaMap,
}

pub fn analyze_all<P: Process + MemoryView>(process: &mut P) -> Result<AnalysisResult> {
    let buttons = analyze(process, buttons);

    info!("found {} buttons", buttons.len());

    let interfaces = analyze(process, interfaces);

    info!(
        "found {} interfaces across {} modules",
        interfaces
            .iter()
            .map(|(_, ifaces)| ifaces.len())
            .sum::<usize>(),
        interfaces.len()
    );

    let offsets = analyze(process, offsets);

    info!(
        "found {} offsets across {} modules",
        offsets
            .iter()
            .map(|(_, offsets)| offsets.len())
            .sum::<usize>(),
        offsets.len()
    );

    let schemas = analyze(process, schemas);

    let (class_count, enum_count) =
        schemas
            .values()
            .fold((0, 0), |(classes, enums), (class_vec, enum_vec)| {
                (classes + class_vec.len(), enums + enum_vec.len())
            });

    info!(
        "found {} classes and {} enums across {} modules",
        class_count,
        enum_count,
        schemas.len()
    );

    Ok(AnalysisResult {
        buttons,
        interfaces,
        offsets,
        schemas,
    })
}

fn analyze<P, F, T>(process: &mut P, f: F) -> T
where
    P: Process + MemoryView,
    F: FnOnce(&mut P) -> Result<T>,
    T: Default,
{
    let name = type_name::<F>();

    match f(process) {
        Ok(result) => result,
        Err(err) => {
            error!("failed to read {}: {}", name, err);

            T::default()
        }
    }
}
