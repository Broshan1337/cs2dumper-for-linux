use std::collections::BTreeMap;
use std::mem;
use std::str::FromStr;

use anyhow::{Result, anyhow};

use log::{debug, error};

use memflow::prelude::v1::*;

use crate::config::{CONFIG, Operation, Signature};

pub type OffsetMap = BTreeMap<String, BTreeMap<String, u32>>;

pub fn offsets<P: Process + MemoryView>(process: &mut P) -> Result<OffsetMap> {
    let mut map = BTreeMap::new();

    for (module_name, sigs) in CONFIG.signatures.iter().flatten() {
        let module = process.module_by_name(module_name)?;

        let mut offsets = BTreeMap::new();

        for sig in sigs {
            match read_offset(process, &module, sig) {
                Ok(value) => {
                    offsets.insert(sig.name.clone(), value);
                }
                Err(err) => error!("{}", err),
            }
        }

        if !offsets.is_empty() {
            map.insert(module_name.to_string(), offsets);
        }
    }

    Ok(map)
}

fn read_offset<P: Process + MemoryView>(
    process: &mut P,
    module: &ModuleInfo,
    signature: &Signature,
) -> Result<u32> {
    let buf = process
        .read_raw(module.base, module.size as _)
        .data_part()?;

    let addr = skidscan::Signature::from_str(&signature.pattern)
        .map_err(|_| anyhow!("unable to parse signature: {}", signature.name))?
        .scan(&buf)
        .ok_or_else(|| anyhow!("unable to find signature for: {}", signature.name))?;

    let mut result = module.base + addr;

    for op in &signature.operations {
        result = match op {
            Operation::Add { value } => result + *value,
            Operation::Rip { offset, len } => {
                let offset: i32 = process.read(result + offset.unwrap_or(0x3)).data_part()?;

                (result + offset) + len.unwrap_or(7)
            }
            Operation::Read => process.read_addr64(result).data_part()?,
            Operation::Slice { start, end } => {
                let buf = process.read_raw(result + *start, end - start).data_part()?;

                let mut bytes = [0; mem::size_of::<usize>()];

                bytes[..buf.len()].copy_from_slice(&buf);

                usize::from_le_bytes(bytes).into()
            }
            Operation::Sub { value } => result - *value,
        };
    }

    let value = (result - module.base)
        .try_into()
        .unwrap_or_else(|_| result.to_umem() as u32);

    debug!(
        "found \"{}\" at {:#X} ({} + {:#X})",
        signature.name,
        value as u64 + module.base.to_umem(),
        module.name,
        value
    );

    Ok(value)
}

#[cfg(test)]
mod tests {
    use std::fs;
    use std::sync::Once;

    use serde_json::Value;

    use simplelog::*;

    use super::*;

    #[test]
    fn build_number() -> Result<()> {
        let mut process = setup()?;

        let engine_base = process.module_by_name("libengine2.so")?.base;

        let offset = read_offset("libengine2.so", "dwBuildNumber").unwrap();

        let build_number: u32 = process.read(engine_base + offset).data_part()?;

        debug!("build number: {}", build_number);

        Ok(())
    }

    #[test]
    fn global_vars() -> Result<()> {
        let mut process = setup()?;

        let client_base = process.module_by_name("libclient.so")?.base;

        let offset = read_offset("libclient.so", "dwGlobalVars").unwrap();

        let global_vars: u64 = process.read(client_base + offset).data_part()?;

        let map_name_addr = process
            .read_addr64((global_vars + 0x180).into())
            .data_part()?;

        let map_name = process.read_utf8(map_name_addr, 128).data_part()?;

        debug!("[global vars] map name: \"{}\"", map_name);

        Ok(())
    }

    #[test]
    fn local_controller() -> Result<()> {
        let mut process = setup()?;

        let client_base = process.module_by_name("libclient.so")?.base;

        let local_controller_offset = read_offset("libclient.so", "dwLocalPlayerController").unwrap();

        let player_name_offset =
            read_class_field("libclient.so", "CBasePlayerController", "m_iszPlayerName").unwrap();

        let local_controller: u64 = process
            .read(client_base + local_controller_offset)
            .data_part()?;

        let player_name = process
            .read_utf8((local_controller + player_name_offset).into(), 128)
            .data_part()?;

        debug!("[local controller] name: \"{}\"", player_name);

        Ok(())
    }

    #[test]
    fn local_pawn() -> Result<()> {
        #[derive(Pod)]
        #[repr(C)]
        struct Vector3D {
            x: f32,
            y: f32,
            z: f32,
        }

        let mut process = setup()?;

        let client_base = process.module_by_name("libclient.so")?.base;

        let local_player_pawn_offset = read_offset("libclient.so", "dwLocalPlayerPawn").unwrap();

        let game_scene_node_offset =
            read_class_field("libclient.so", "C_BaseEntity", "m_pGameSceneNode").unwrap();

        let origin_offset =
            read_class_field("libclient.so", "CGameSceneNode", "m_vecAbsOrigin").unwrap();

        let local_player_pawn: u64 = process
            .read(client_base + local_player_pawn_offset)
            .data_part()?;

        let game_scene_node: u64 = process
            .read((local_player_pawn + game_scene_node_offset).into())
            .data_part()?;

        let origin: Vector3D = process
            .read((game_scene_node + origin_offset).into())
            .data_part()?;

        debug!(
            "[local pawn] origin: {:.2}, y: {:.2}, z: {:.2}",
            origin.x, origin.y, origin.z
        );

        Ok(())
    }

    #[test]
    fn window_size() -> Result<()> {
        let mut process = setup()?;

        let engine_base = process.module_by_name("libengine2.so")?.base;

        let window_width_offset = read_offset("libengine2.so", "dwWindowWidth").unwrap();
        let window_height_offset = read_offset("libengine2.so", "dwWindowHeight").unwrap();

        let window_width: u32 = process
            .read(engine_base + window_width_offset)
            .data_part()?;

        let window_height: u32 = process
            .read(engine_base + window_height_offset)
            .data_part()?;

        debug!("window size: {}x{}", window_width, window_height);

        Ok(())
    }

    fn setup() -> Result<IntoProcessInstanceArcBox<'static>> {
        static LOGGER: Once = Once::new();

        LOGGER.call_once(|| {
            SimpleLogger::init(LevelFilter::Trace, Config::default()).ok();
        });

        let os = memflow_native::create_os(&OsArgs::default(), LibArc::default())?;

        let process = os.into_process_by_name(&CONFIG.executable)?;

        Ok(process)
    }

    fn read_class_field(module_name: &str, class_name: &str, field_name: &str) -> Option<u64> {
        let content =
            fs::read_to_string(format!("output/{}.json", module_name.replace(".", "_"))).ok()?;

        let value: Value = serde_json::from_str(&content).ok()?;

        value
            .get(module_name)?
            .get("classes")?
            .get(class_name)?
            .get("fields")?
            .get(field_name)?
            .as_u64()
    }

    fn read_offset(module_name: &str, offset_name: &str) -> Option<u64> {
        let content = fs::read_to_string("output/offsets.json").ok()?;
        let value: Value = serde_json::from_str(&content).ok()?;

        let offset = value.get(module_name)?.get(offset_name)?;

        offset.as_u64()
    }
}
