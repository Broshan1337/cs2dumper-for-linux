#!/usr/bin/env python3
"""Check the signatures in config.json against the installed CS2 binaries.

Scans each module for its byte pattern, then replays the operation chain
(rip/read/add/sub/slice) to show the offset each signature currently produces --
without attaching to the running game. Run it after a game update to see what
broke before doing a live dump.

    tools/verify_signatures.py [--game-dir DIR]

Exits non-zero if any signature no longer matches.
"""

import argparse
import json
import os
import re
import struct
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def steam_libraries():
    """Every Steam library root listed in libraryfolders.vdf, plus the defaults."""
    roots, seen = [], set()
    for base in (
        os.path.expanduser("~/.local/share/Steam"),
        os.path.expanduser("~/.steam/steam"),
        os.path.expanduser("~/.var/app/com.valvesoftware.Steam/.local/share/Steam"),
    ):
        vdf = os.path.join(base, "steamapps", "libraryfolders.vdf")
        if not os.path.exists(vdf):
            continue
        for path in re.findall(r'"path"\s+"([^"]+)"', open(vdf, encoding="utf8", errors="replace").read()):
            if path not in seen:
                seen.add(path)
                roots.append(path)
    return roots


def find_game_dir():
    for root in steam_libraries():
        cand = os.path.join(root, "steamapps", "common", "Counter-Strike Global Offensive", "game")
        if os.path.isdir(cand):
            return cand
    return None


class Module:
    """A module image, indexed by virtual address like the dumper sees it."""

    def __init__(self, path):
        self.path = path
        self.data = open(path, "rb").read()
        self.segs, self.rel, self.secs = [], {}, []

        for line in self._readelf("-lW"):
            f = line.split()
            if len(f) >= 6 and f[0] == "LOAD":
                self.segs.append((int(f[1], 16), int(f[2], 16), int(f[4], 16)))
        for line in self._readelf("-rW"):
            f = line.split()
            if len(f) >= 4 and f[2] == "R_X86_64_RELATIVE":
                try:
                    self.rel[int(f[0], 16)] = int(f[3], 16)
                except ValueError:
                    pass
        for line in self._readelf("-SW"):
            if "]" not in line:
                continue
            f = line.split("]", 1)[1].split()
            if len(f) >= 5:
                try:
                    self.secs.append((f[0], int(f[2], 16), int(f[4], 16)))
                except ValueError:
                    pass

    def _readelf(self, flag):
        return subprocess.run(["readelf", flag, self.path], capture_output=True, text=True).stdout.splitlines()

    def file_to_vaddr(self, off):
        for fo, va, sz in self.segs:
            if fo <= off < fo + sz:
                return va + (off - fo)

    def vaddr_to_file(self, va):
        for fo, v, sz in self.segs:
            if v <= va < v + sz:
                return fo + (va - v)

    def i32(self, va):
        return struct.unpack("<i", self.data[self.vaddr_to_file(va):][:4])[0]

    def ptr(self, va):
        # Statically initialised pointers live in the relocation table, not the file.
        if va in self.rel:
            return self.rel[va]
        return struct.unpack("<Q", self.data[self.vaddr_to_file(va):][:8])[0]

    def raw(self, va, n):
        return self.data[self.vaddr_to_file(va):][:n]

    def section(self, va):
        for name, addr, size in self.secs:
            if addr <= va < addr + size:
                return name
        return "??"


def to_regex(pattern):
    out = b""
    for tok in pattern.split():
        out += b"." if tok in ("?", "??") else re.escape(bytes([int(tok, 16)]))
    return re.compile(out, re.DOTALL)


def resolve(mod, pattern, operations):
    hits = [m.start() for m in to_regex(pattern).finditer(mod.data)]
    if not hits:
        return None, 0

    cur = mod.file_to_vaddr(hits[0])
    for op in operations:
        kind = op["type"]
        if kind == "rip":
            cur += mod.i32(cur + op.get("offset", 3)) + op.get("length", op.get("len", 7))
        elif kind == "read":
            cur = mod.ptr(cur)
        elif kind == "add":
            cur += op["value"]
        elif kind == "sub":
            cur -= op["value"]
        elif kind == "slice":
            cur = int.from_bytes(mod.raw(cur + op["start"], op["end"] - op["start"]).ljust(8, b"\0"), "little")
    return cur, len(hits)


# Signatures compiled into src/ rather than config.json. Interfaces are absent by
# design: they resolve the exported CreateInterface symbol and scan only that
# function's prologue, so there is no module-wide pattern to verify.
BUILTIN = [
    ("libclient.so", "buttons list  [src]",
     "48 8B 05 ? ? ? ? 48 89 1D ? ? ? ? 48 89 83 88 00 00 00",
     [{"type": "rip"}]),
]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--game-dir", help="CS2 'game' directory (auto-detected from Steam if omitted)")
    ap.add_argument("--config", default=os.path.join(REPO, "config.json"))
    args = ap.parse_args()

    game = args.game_dir or find_game_dir()
    if not game or not os.path.isdir(game):
        print("error: could not locate the CS2 install; pass --game-dir", file=sys.stderr)
        return 2

    search = [os.path.join(game, "bin", "linuxsteamrt64"),
              os.path.join(game, "csgo", "bin", "linuxsteamrt64")]

    def find_so(name):
        for d in search:
            p = os.path.join(d, name)
            if os.path.exists(p):
                return p

    cfg = json.load(open(args.config))
    mods, rows = {}, []

    for group in cfg["signatures"]:
        for module, sigs in group.items():
            path = find_so(module)
            if not path:
                rows.append((module, "<module not found>", None, 0))
                continue
            mods.setdefault(module, Module(path))
            for s in sigs:
                value, n = resolve(mods[module], s["pattern"], s["operations"])
                rows.append((module, s["name"], value, n))

    for module, label, pattern, ops in BUILTIN:
        path = find_so(module)
        if not path:
            continue
        mods.setdefault(module, Module(path))
        value, n = resolve(mods[module], pattern, ops)
        rows.append((module, label, value, n))

    print(f"{'module':<20} {'signature':<40} {'offset':>11}  {'section':<16} status")
    print("-" * 96)
    missing = ambiguous = 0
    for module, name, value, n in rows:
        if value is None:
            section = "-"
        elif value > 0x10000:
            section = mods[module].section(value)
        else:
            section = "(member offset)"
        status = "OK" if n == 1 else (f"AMBIGUOUS({n})" if n else "MISSING")
        if n == 0:
            missing += 1
        elif n > 1:
            ambiguous += 1
        shown = f"0x{value:X}" if value is not None else "-"
        print(f"{module:<20} {name:<40} {shown:>11}  {section:<16} {status}")

    print("-" * 96)
    print(f"total={len(rows)}  missing={missing}  ambiguous={ambiguous}")
    if ambiguous:
        print("\nnote: an ambiguous signature is not necessarily broken -- several matches may")
        print("      resolve to the same address. Check the offset column before changing it.")
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(main())
