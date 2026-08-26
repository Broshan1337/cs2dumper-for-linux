# cs2-dumper (Linux)

An external offset/interface/schema dumper for Counter-Strike 2 on Linux. Powered by
[memflow](https://github.com/memflow/memflow).

This is a fork of [a2x/cs2-dumper](https://github.com/a2x/cs2-dumper), based on its
[`linux`](https://github.com/a2x/cs2-dumper/tree/linux) branch. Upstream notes that branch is not
actively maintained ("this branch will likely not be kept up-to-date by myself. Pull requests are
welcome!"), and it had fallen a long way behind `main`. This fork brings it back up to date: it builds
on current Rust, matches `main`'s output format, and dumps cleanly against the current CS2 build.

**Status:** verified against CS2 build `14178` (PatchVersion 1.41.7.8)

## What was updated

**Signatures.** Most of the upstream branch's signatures no longer matched the game and it produced
largely broken output. Re-derived against the current binaries:

| Signature | Was | Now |
| --- | --- | --- |
| button list | matched two unrelated sites; dumped one empty entry | anchored on the `KeyButton` registrar |
| interfaces | pattern matched a decoy in `libclient.so`; garbled names | resolved from the exported `CreateInterface` symbol |
| `dwBuildNumber` | three matches, all inside `mov qword` REX prefixes | anchored on the `steam.inf` `PatchVersion` store |
| `dwNetworkGameClient` | zero matches (embedded a build-specific call offset) | follows the engine service object to the client pointer |
| `dwPlantedC4` | 29 matches | anchored on the `CUtlAutoList<C_PlantedC4>` head |

**Interface discovery** no longer pattern-scans whole modules. It resolves the exported
`CreateInterface` symbol and scans only that function's prologue, which is unambiguous and mirrors how
`main` uses the PE export table on Windows. This needs a small ELF dynamic-symbol reader
([`src/memory/elf.rs`](src/memory/elf.rs)) because memflow's own export lookup is unreliable on these
modules: it locates `PT_DYNAMIC` by file offset but resolves `DT_SYMTAB`/`DT_STRTAB` by virtual
address, which disagree here.

**Parity with `main`.** Zig output (previously missing), `main`'s map-of-maps JSON shape, slugified
output filenames, `--connector` / `--connector-args` / `--process-name` / `--no-log-file`, a log file,
per-phase resilience so one broken signature no longer aborts the whole dump, and per-phase summary
counts.

**Toolchain.** Rust 2024 edition, `anyhow` in place of the hand-rolled error enum, current memflow
idioms, and an unused `phf` dependency dropped. Builds warning-free on stable.

## Building

Requires Rust 1.85 or newer (2024 edition). The stable toolchain is sufficient.

```sh
cargo build --release
```

## Usage

1. Start CS2 and get to the main menu — that is enough, you do not need to be in a match.
2. Run the dumper from the repository root (`config.json` and `output/` are resolved from the working
   directory):

```sh
./target/release/cs2-dumper -vv
```

No elevated privileges are needed as long as `/proc/sys/kernel/yama/ptrace_scope` is `0` and the game
runs as the same user. If it is `1` or higher, run with `sudo`.

By default the dumper uses the [memflow-native](https://github.com/memflow/memflow-native) OS layer to
read the game's memory. To use a memflow connector such as **pcileech** or **kvm** instead, pass
`--connector` and optionally `--connector-args`; these can be installed with
[memflowup](https://github.com/memflow/memflowup). Some connectors require elevated privileges.

```sh
./target/release/cs2-dumper -c pcileech -a :device=FPGA -vv
```

### Arguments

- `-c, --connector <connector>`: The name of the memflow connector to use.
- `-a, --connector-args <connector-args>`: Additional arguments to pass to the memflow connector.
- `-f, --file-types <file-types>`: The types of files to generate. Default: `cs`, `hpp`, `json`, `rs`, `zig`.
- `-i, --indent-size <indent-size>`: The number of spaces to use per indentation level. Default: `4`.
- `-o, --output <output>`: The output directory to write the generated files to. Default: `output`.
- `-p, --process-name <process-name>`: The name of the game process. Defaults to the `executable` field
  in `config.json` (`cs2`).
- `-v, --verbose...`: Increase logging verbosity. Can be specified multiple times. `-vvv` logs every
  resolved symbol by name, which is the quickest way to spot a bad dump.
- `-n, --no-log-file`: Prevent creation of the `cs2-dumper.log` file.
- `-h, --help` / `-V, --version`.

Note that `cs2-dumper.log` is always written at info level regardless of `-v`; to keep the verbose
output, redirect it (`-vvv 2>&1 | tee dump.log`).

## Signatures

Unlike the Windows version, which compiles its patterns in, this branch reads them from
[`config.json`](config.json) at runtime. Each signature names a module, a byte pattern, and a list of
operations applied in order to turn the match into a module-relative offset:

| Operation | Effect |
| --- | --- |
| `rip` | Resolve a RIP-relative displacement. `offset` defaults to 3, `length` to 7. |
| `read` | Dereference the current address. |
| `add` / `sub` | Adjust the current address by `value`. |
| `slice` | Read bytes `start..end` and interpret them as a little-endian integer. |

Interfaces are not listed there — they are resolved from the `CreateInterface` export instead.

### After a game update

```sh
tools/verify_signatures.py
```

This checks every signature against the installed binaries and replays its operation chain, showing
the offset each currently resolves to, without attaching to the game. It exits non-zero if anything no
longer matches, so it drops into a script. An `AMBIGUOUS` result is not necessarily a failure —
several matches can resolve to the same address — so check the offset column before changing anything.

## Output

`output/` holds the generated `cs`, `hpp`, `json`, `rs`, and `zig` files, plus `info.json` with the
build number and a timestamp. Module names are slugified, so `libclient.so` becomes `libclient_so.*`.

## Credits

Original project and all of its design by [a2x](https://github.com/a2x). Licensed under the MIT
license ([LICENSE](LICENSE)); the copyright notice is unchanged.
