#![allow(dead_code)]
#![allow(unused_imports)]

use std::fs::File;
use std::path::PathBuf;
use std::str::FromStr;
use std::time::Instant;

use anyhow::Result;

use clap::{ArgAction, Parser};

use log::{LevelFilter, info};

use memflow::prelude::v1::*;

use simplelog::*;

use config::CONFIG;
use output::Output;

mod analysis;
mod config;
mod memory;
mod output;
mod source2;

#[derive(Debug, Parser)]
#[command(author, version)]
struct Args {
    /// The name of the memflow connector to use.
    #[arg(short, long)]
    connector: Option<String>,

    /// Additional arguments to pass to the memflow connector.
    #[arg(short = 'a', long)]
    connector_args: Option<String>,

    /// The types of files to generate.
    #[arg(
        short,
        long,
        value_delimiter = ',',
        default_values = ["cs", "hpp", "json", "rs", "zig"]
    )]
    file_types: Vec<String>,

    /// The number of spaces to use per indentation level.
    #[arg(short, long, default_value_t = 4)]
    indent_size: usize,

    /// The output directory to write the generated files to.
    #[arg(short, long, default_value = "output")]
    output: PathBuf,

    /// The name of the game process. Defaults to the `executable` field in config.json.
    #[arg(short, long)]
    process_name: Option<String>,

    /// Increase logging verbosity. Can be specified multiple times.
    #[arg(short, long, action = ArgAction::Count)]
    verbose: u8,

    /// Prevent creation of the cs2-dumper.log file.
    #[arg(short, long)]
    no_log_file: bool,
}

fn main() -> Result<()> {
    let args = Args::parse();

    let level_filter = match args.verbose {
        0 => LevelFilter::Error,
        1 => LevelFilter::Warn,
        2 => LevelFilter::Info,
        3 => LevelFilter::Debug,
        _ => LevelFilter::Trace,
    };

    let mut loggers: Vec<Box<dyn SharedLogger>> = vec![TermLogger::new(
        level_filter,
        Config::default(),
        TerminalMode::Mixed,
        ColorChoice::Auto,
    )];

    // Create the log file by default.
    if !args.no_log_file {
        loggers.push(WriteLogger::new(
            LevelFilter::Info,
            Config::default(),
            File::create("cs2-dumper.log")?,
        ));
    }

    CombinedLogger::init(loggers)?;

    let conn_args = args
        .connector_args
        .map(|s| ConnectorArgs::from_str(&s).expect("unable to parse connector arguments"))
        .unwrap_or_default();

    let mut os = match args.connector {
        Some(conn) => {
            let mut inventory = Inventory::scan();

            inventory
                .builder()
                .connector(&conn)
                .args(conn_args)
                .os("linux")
                .build()?
        }
        None => memflow_native::create_os(&OsArgs::default(), LibArc::default())?,
    };

    let process_name = args.process_name.as_deref().unwrap_or(&CONFIG.executable);

    let mut process = os.process_by_name(process_name)?;

    let now = Instant::now();

    let result = analysis::analyze_all(&mut process)?;
    let output = Output::new(&args.file_types, args.indent_size, &args.output, &result)?;

    output.dump_all(&mut process)?;

    info!("analysis completed in {:.2?}", now.elapsed());

    Ok(())
}
