use std::collections::HashMap;
use std::path::PathBuf;
use std::sync::LazyLock;
use std::{env, fs, process};

use serde::{Deserialize, Serialize};

pub static CONFIG: LazyLock<Config> = LazyLock::new(|| {
    let path = config_path().unwrap_or_else(|| {
        eprintln!(
            "error: config.json not found.\n\
             \n\
             Looked in the current directory and next to the executable. It ships alongside\n\
             the binary, so keep the two together, or run the dumper from the repository root."
        );

        process::exit(1);
    });

    let content = fs::read_to_string(&path).unwrap_or_else(|err| {
        eprintln!("error: unable to read {}: {}", path.display(), err);

        process::exit(1);
    });

    serde_json::from_str(&content).unwrap_or_else(|err| {
        eprintln!("error: unable to parse {}: {}", path.display(), err);

        process::exit(1);
    })
});

/// Looks for `config.json` in the working directory first, then next to the executable,
/// so an extracted release archive works regardless of where it is run from.
fn config_path() -> Option<PathBuf> {
    let cwd = PathBuf::from("config.json");

    if cwd.is_file() {
        return Some(cwd);
    }

    let beside_exe = env::current_exe().ok()?.parent()?.join("config.json");

    beside_exe.is_file().then_some(beside_exe)
}

#[derive(Debug, Deserialize, Serialize)]
#[serde(rename_all = "snake_case", tag = "type")]
pub enum Operation {
    /// Adds the specified value to the current address.
    Add { value: usize },

    /// Resolves the absolute address of a RIP-relative address.
    Rip {
        /// The offset of the displacement value.
        offset: Option<usize>,

        /// The total length of the instruction.
        #[serde(alias = "length")]
        len: Option<usize>,
    },

    /// Reads the value at the current address, treating it as a pointer.
    Read,

    /// Extracts a range of bytes from the current address and interprets them as a value.
    Slice { start: usize, end: usize },

    /// Subtracts the specified value from the current address.
    Sub { value: usize },
}

#[derive(Debug, Deserialize, Serialize)]
pub struct Config {
    pub executable: String,
    pub signatures: Vec<HashMap<String, Vec<Signature>>>,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct Signature {
    pub name: String,
    pub pattern: String,
    pub operations: Vec<Operation>,
}
