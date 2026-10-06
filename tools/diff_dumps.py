#!/usr/bin/env python3
"""Diff two cs2-dumper output directories (old vs new game build).

This is the per-update triage tool: run the dumper once per update, then

    tools/diff_dumps.py output_old output_new

and read the report top-down. Sections:
  - signatures/offsets: which dw* globals moved, appeared, disappeared
  - interfaces / buttons: same
  - schema: per-module class/field drift (renames, offset changes, adds, removals)
  - vtables: per class, slot-count changes and per-slot function shifts
    (a uniform shift = recompile, expected; a same-count non-uniform shift
     or a count change = slot semantics may have moved - re-verify hooks)

Exit code 0 when no differences, 1 otherwise (scriptable).
"""

import json
import sys
from pathlib import Path


def load(base: Path, name: str):
    p = base / name
    if not p.exists():
        return None
    return json.loads(p.read_text())


def fmt(x):
    if isinstance(x, int):
        return hex(x)
    return str(x)


def diff_flat(label, old, new):
    """old/new: {module: {name: value}}"""
    lines = []
    if old is None or new is None:
        print(f"## {label}: {'NEW' if old is None else 'MISSING OLD'} dump")
        return
    for module in sorted(set(old) | set(new)):
        om, nm = old.get(module, {}), new.get(module, {})
        changed, added, removed = [], [], []
        for k in sorted(set(nm) | set(om)):
            if k not in om:
                added.append(k)
            elif k not in nm:
                removed.append(k)
            elif om[k] != nm[k]:
                changed.append((k, om[k], nm[k]))
        if changed or added or removed:
            lines.append(f"  {module}: {len(changed)} moved, {len(added)} added, {len(removed)} removed")
            for k, a, b in changed:
                lines.append(f"    {k}: {fmt(a)} -> {fmt(b)} (shift {hex(b - a) if isinstance(a, int) and isinstance(b, int) else '?'})")
            for k in added:
                lines.append(f"    + {k} = {fmt(nm[k])}")
            for k in removed:
                lines.append(f"    - {k}")
    if lines:
        print(f"## {label}")
        print("\n".join(lines))
    else:
        print(f"## {label}: unchanged")


def shift_pattern(values):
    """values: list of int deltas -> 'uniform 0x…' / 'mixed'"""
    uniq = sorted(set(values))
    if len(uniq) == 1:
        return f"uniform {hex(uniq[0])}"
    if len(uniq) <= 4:
        return "mixed " + " ".join(hex(v) for v in uniq)
    return f"{len(uniq)} distinct shifts"


def diff_vtables(old, new):
    if old is None or new is None:
        print("## vtables: dump missing on one side (feature added later?)")
        return
    header = False
    for module in sorted(set(old) | set(new)):
        om, nm = old.get(module, {}), new.get(module, {})
        gone = [c for c in om if c not in nm]
        added = [c for c in nm if c not in om]
        if gone or added:
            print(f"## vtables/{module}: {len(added)} new classes, {len(gone)} removed")
            for c in gone[:10]:
                print(f"  - {c}")
            for c in added[:10]:
                print(f"  + {c}")
            header = True
        slot_count_changes = []
        reshuffles = []
        shifts = []
        for cls in nm:
            if cls not in om:
                continue
            o, n = om[cls], nm[cls]
            os_, ns_ = o["slots"], n["slots"]
            if len(os_) != len(ns_):
                slot_count_changes.append((cls, len(os_), len(ns_)))
                continue
            deltas = [b - a for a, b in zip(os_, ns_)]
            if all(d == 0 for d in deltas):
                continue
            if all(d == deltas[0] for d in deltas):
                shifts.append((cls, deltas[0]))
            else:
                # per-slot shifts differ -> in-place recompile or semantic change
                idx = [i for i, d in enumerate(deltas) if d != deltas[0]]
                reshuffles.append((cls, deltas[0], idx[:8], ns_))
        if shifts:
            vals = [d for _, d in shifts]
            print(f"## vtables/{module}: {len(shifts)} classes uniformly shifted ({shift_pattern(vals)}), "
                  f"{len(slot_count_changes)} slot-count changes, {len(reshuffles)} non-uniform")
            for cls, cnt_o, cnt_n in slot_count_changes[:10]:
                print(f"  SLOT COUNT {cls}: {cnt_o} -> {cnt_n}  << re-derive slots for this class")
            for cls, base_d, idx, ns_ in reshuffles[:10]:
                print(f"  NON-UNIFORM {cls}: base shift {hex(base_d)}, differing slots {idx} << re-verify")
            header = True
    if not header:
        print("## vtables: unchanged")


def diff_schemas(old, new):
    if old is None or new is None:
        print("## schema: dump missing on one side")
        return
    for module in sorted(set(old) | set(new)):
        om = {c: old[module][c] for c in old.get(module, {})} if module in old else {}
        nm = {c: new[module][c] for c in new.get(module, {})} if module in new else {}
        changes, added, removed = [], [], []
        for c in nm:
            if c not in om:
                added.append(c)
                continue
            of, nf = om[c].get("fields", {}), nm[c].get("fields", {})
            for f, v in nf.items():
                if f in of and of[f] != v:
                    changes.append((c, f, of[f], v))
            for f in of:
                if f not in nf:
                    removed.append((c, f))
        if added or removed or changes:
            print(f"## schema/{module}: {len(changes)} field changes, {len(added)} new classes, {len(removed)} removed fields")
            for c, f, a, b in changes[:15]:
                print(f"  {c}.{f}: {fmt(a)} -> {fmt(b)}")
            if len(changes) > 15:
                print(f"  ... and {len(changes) - 15} more")
            for c in added[:10]:
                print(f"  + class {c}")
            for c, f in removed[:10]:
                print(f"  - field {c}.{f}")


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        return 2
    old_dir, new_dir = Path(sys.argv[1]), Path(sys.argv[2])

    diff_flat("offsets", load(old_dir, "offsets.json"), load(new_dir, "offsets.json"))
    diff_flat("interfaces", load(old_dir, "interfaces.json"), load(new_dir, "interfaces.json"))
    diff_flat("buttons", load(old_dir, "buttons.json"), load(new_dir, "buttons.json"))
    diff_vtables(load(old_dir, "vtables.json"), load(new_dir, "vtables.json"))

    # per-module schema files: libclient_so.json etc.
    old_files = {p.name: p for p in old_dir.glob("*_so.json")}
    new_files = {p.name: p for p in new_dir.glob("*_so.json")}
    for name in sorted(set(old_files) | set(new_files)):
        o = json.loads(old_files[name].read_text()) if name in old_files else None
        n = json.loads(new_files[name].read_text()) if name in new_files else None
        # unwrap {module: {classes, enums}}
        if o and list(o)[0].endswith(".so"):
            o = o[list(o)[0]]["classes"]
            n = n[list(n)[0]]["classes"] if n else None
        diff_schemas(o, n)

    return 0


if __name__ == "__main__":
    sys.exit(main())
