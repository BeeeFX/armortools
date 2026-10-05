"""The layer's smoke test (layer/README.md > Testing): the layer's files checked against this tree's upstream sources,
then the built ArmorPaint run headless.

    python layer/tests/smoke/smoke.py                          # the build in ~/.singerie/tools/armorpaint-layer-build
    python layer/tests/smoke/smoke.py --armorpaint <folder>    # another build (the folder holding ArmorPaint.exe)
    python layer/tests/smoke/smoke.py --static                 # only the checks that need no build
    python layer/tests/smoke/smoke.py --update-snapshot        # after reviewing an upstream API change

Without a build (checked against upstream's sources in this tree):
- the theme follows ArmorPaint's theme fields in their order (its theme reader is positional, not by name);
- the keymap preset only names actions ArmorPaint has (an upstream rename would silently drop a shortcut);
- the export presets have upstream's shape, only channel ids ArmorPaint exports, and names that say their colour
  space as Singerie's rule wants (`basecolor_srgb`, `roughness_raw`...); no layer file replaces an upstream one.

With the build (each run in a copy of its folder, so the build stays untouched):
- the layer's data files are in the app's data folder;
- `--api`: every sng_* function registered, and ArmorPaint's own script API (functions, structs, enums, globals and
  the material nodes) compared with `api_snapshot.h`, so an upstream API change is caught;
- the first start of a fresh copy gives it the Painter keymap and theme, written by ArmorPaint in its config.json;
  a config with the member's own choice (ArmorPaint's default keymap and theme) keeps it;
- `scripts/bindings.c`: every sng_* function on a fresh project, then an export with the studio preset, whose files
  are checked (names, PNG, size).

Exit status 0 when everything passed. Standard library only (Python 3.10+).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
LAYER = HERE.parents[1]
REPO = LAYER.parent
SNAPSHOT = HERE / "api_snapshot.h"
DEFAULT_APP = Path.home() / ".singerie" / "tools" / "armorpaint-layer-build" / "armorpaint"
EXT_FOLDERS = ("keymap_presets", "themes", "export_presets", "plugins")
TIMEOUT = 180  # seconds for one ArmorPaint run

# Singerie's colour rule for map names (singerie_core.texture_sets / colour_names): colour maps say srgb, data raw.
COLOUR_CHANNELS = {"basecolor", "diffuse", "specular", "emission"}
CONSTANTS = {"0.0", "1.0"}

failures: list[str] = []


def check(what: str, ok: bool, detail: object = "") -> bool:
    print(f"{'ok  ' if ok else 'FAIL'} {what}" + (f": {detail}" if detail != "" and not ok else ""), flush=True)
    if not ok:
        failures.append(what)
    return ok


# ████ Without a build ████


def upstream_text(relative: str) -> str:
    return (REPO / relative).read_text(encoding="utf-8", errors="replace")


def theme_keys() -> list[str]:
    """ArmorPaint's theme fields in order: ui_theme_keys in base/sources/iron_ui_ext.c."""
    found = re.search(r"ui_theme_keys\[\]\s*=\s*\{(.*?)\};", upstream_text("base/sources/iron_ui_ext.c"), re.S)
    return re.findall(r'"(\w+)"', found.group(1)) if found else []


def keymap_actions() -> set[str]:
    """Every action of ArmorPaint's default keymap (keymap_get_default in paint/sources/keymap.c)."""
    return set(re.findall(r'any_map_set\(keymap,\s*"(\w+)"', upstream_text("paint/sources/keymap.c")))


def export_channel_ids() -> set[str]:
    """The channel ids ArmorPaint's exporter knows (paint/sources/io/export_texture.c)."""
    return set(re.findall(r'string_equals\(c(?:->buffer\[\d\])?,\s*"(\w+)"\)', upstream_text("paint/sources/io/export_texture.c")))


def ordered_keys(text: str) -> list[list[str]]:
    """The keys of each JSON object in a file, in the order written (json.loads would lose nothing here, but
    ArmorPaint's reader is positional, so the order is what matters)."""
    keys: list[list[str]] = []

    def hook(pairs):
        keys.append([key for key, _ in pairs])
        return dict(pairs)

    json.loads(text, object_pairs_hook=hook)
    return keys


def static_checks() -> None:
    print("== The layer's files, against this tree's upstream sources", flush=True)
    ext = LAYER / "ext"
    for folder in EXT_FOLDERS:
        for path in sorted((ext / folder).glob("*")) if (ext / folder).is_dir() else []:
            upstream = REPO / "paint" / "assets" / folder / path.name
            check(f"{folder}/{path.name} doesn't replace an upstream file", not upstream.exists())

    # Theme: every field, in ArmorPaint's order, all integers.
    expected = theme_keys()
    check("ArmorPaint's theme fields found (ui_theme_keys)", len(expected) > 0)
    for path in sorted((ext / "themes").glob("*.json")):
        text = path.read_text(encoding="utf-8")
        keys = ordered_keys(text)[-1]
        check(f"themes/{path.name} has ArmorPaint's {len(expected)} fields in their order", keys == expected, keys)
        values = json.loads(text)
        check(f"themes/{path.name} holds whole numbers only", all(type(v) is int for v in values.values()))

    # Keymap: actions ArmorPaint has, and no plain key shared by two actions (ArmorPaint would fire both).
    actions = keymap_actions()
    check("ArmorPaint's keymap actions found (keymap_get_default)", len(actions) > 40, len(actions))
    for path in sorted((ext / "keymap_presets").glob("*.json")):
        preset = json.loads(path.read_text(encoding="utf-8"))
        unknown = sorted(set(preset) - actions)
        check(f"keymap_presets/{path.name} names only ArmorPaint's actions", not unknown, unknown)
        merged = {**default_keymap(), **preset}
        plain: dict[str, list[str]] = {}
        for action, key in merged.items():
            if key and "+" not in key and key not in ("left", "right", "middle", "ctrl", "shift", "alt", "number"):
                plain.setdefault(key, []).append(action)
        allowed = {frozenset({"node_overview", "brush_camera_align"}), frozenset({"node_search", "keymap_search"})}
        clashes = {k: v for k, v in plain.items() if len(v) > 1 and frozenset(v) not in allowed}
        check(f"keymap_presets/{path.name} gives no plain key to two viewport actions", not clashes, clashes)

    # Export presets: upstream's shape, known channels, Singerie's names.
    ids = export_channel_ids()
    check("ArmorPaint's export channel ids found (export_texture.c)", len(ids) > 15, len(ids))
    for path in sorted((ext / "export_presets").glob("*.json")):
        text = path.read_text(encoding="utf-8")
        data = json.loads(text)
        objects = ordered_keys(text)
        textures = data.get("textures", [])
        shape = all(keys == ["name", "channels", "color_space"] for keys in objects[:-1]) and objects[-1] == ["textures"]
        check(f"export_presets/{path.name} has upstream's shape (name, channels, color_space)", shape, objects)
        bad = [t["name"] for t in textures if len(t["channels"]) != 4 or not set(t["channels"]) <= ids | CONSTANTS]
        check(f"export_presets/{path.name} uses only channels ArmorPaint exports", not bad, bad)
        not_linear = [t["name"] for t in textures if t["color_space"] != "linear"]
        # "linear" is ArmorPaint's "written as painted"; anything else converts the pixels on export.
        check(f"export_presets/{path.name} writes every map as painted (color_space linear)", not not_linear, not_linear)
        wrong = []
        for texture in textures:
            name = texture["name"]
            word = name.rsplit("_", 1)[-1]
            channel = name.rsplit("_", 1)[0]
            rgb = {c.split("_")[0] for c in texture["channels"][:3]}
            colour = rgb in ({"base"}, {"diff"}, {"spec"})
            if word not in ("srgb", "raw") or (word == "srgb") != colour or (channel in COLOUR_CHANNELS) != colour and channel != "emission_mask":
                wrong.append(name)
        check(f"export_presets/{path.name} names say their colour space (colour srgb, data raw)", not wrong, wrong)


def default_keymap() -> dict[str, str]:
    return dict(re.findall(r'any_map_set\(keymap,\s*"(\w+)",\s*"([^"]*)"\)', upstream_text("paint/sources/keymap.c")))


# ████ With the build ████


def bindings_registered() -> list[str]:
    """The sng_* functions layer/native/sng_bindings.c registers."""
    return re.findall(r'\{"(sng_\w+)",\s*"', (LAYER / "native" / "sng_bindings.c").read_text(encoding="utf-8"))


def api_part(output: str) -> str:
    """The script API and the material nodes reference from `--api`, without the project's state that follows."""
    return output.split("/* Current project state:", 1)[0].rstrip() + "\n"


def api_entries(text: str) -> dict[str, str]:
    """`--api`'s text as entries by name: each struct, enum value, global, function and material node."""
    entries: dict[str, str] = {}
    lines = text.splitlines()
    section = ""
    i = 0
    while i < len(lines):
        line = lines[i]
        if line.startswith("typedef struct "):
            block = [line]
            while i + 1 < len(lines) and not lines[i].startswith("} "):
                i += 1
                block.append(lines[i])
            entries[f"struct {line.split()[2]}"] = "\n".join(block)
        elif line.startswith("#define "):
            entries[f"enum {line.split()[1]}"] = line
        elif line.startswith("extern "):
            entries[f"global {line.rstrip(';').split()[-1].lstrip('*')}"] = line
        elif line.endswith(");") and not line.startswith("//"):
            found = re.search(r"(\w+)\(", line)
            if found:
                entries[f"function {found.group(1)}"] = line
        elif line.startswith("// ") and line.endswith(":"):
            section = line[3:-1]
        elif line.startswith("// ") and not line.startswith("//     ") and section:
            block = [line]
            while i + 1 < len(lines) and lines[i + 1].startswith("//     "):
                i += 1
                block.append(lines[i])
            entries[f"node {section} > {line[3:].split(' | ')[0].strip()}"] = "\n".join(block)
        i += 1
    return entries


def upstream_only(text: str) -> str:
    """The API text without the layer's own functions."""
    return "".join(line for line in text.splitlines(keepends=True) if not re.search(r"\bsng_\w+\(", line))


def run(app: Path, *arguments: str, environment: dict | None = None) -> subprocess.CompletedProcess:
    return subprocess.run(
        [os.fspath(app / "ArmorPaint.exe"), *arguments],
        cwd=app,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        timeout=TIMEOUT,
        env=environment,
        check=False,
    )


def script(tmp: Path, name: str, **values: str) -> Path:
    """A copy of scripts/<name> with its @PLACEHOLDERS@ filled (paths with escaped backslashes, as minic's strings
    need)."""
    text = (HERE / "scripts" / name).read_text(encoding="utf-8")
    for key, value in values.items():
        text = text.replace(f"@{key}@", value.replace("\\", "\\\\"))
    path = tmp / name
    path.write_text(text, encoding="utf-8")
    return path


def read_report(path: Path) -> dict[str, str]:
    if not path.is_file():
        return {}
    return dict(line.split("=", 1) for line in path.read_text(encoding="utf-8").splitlines() if "=" in line)


def png_size(path: Path) -> tuple[int, int] | None:
    data = path.read_bytes()[:24]
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        return None
    return struct.unpack(">II", data[16:24])


def check_api(app: Path, update: bool) -> None:
    print("== --api: the layer's functions, and upstream's script API against the snapshot", flush=True)
    result = run(app, "--api")
    text = api_part(result.stdout)
    if not check("ArmorPaint --api printed its script API", "// ArmorPaint script API" in text, result.returncode):
        return
    entries = api_entries(text)
    found = sorted(name.split()[1] for name in entries if name.startswith("function sng_"))
    expected = sorted(bindings_registered())
    check(f"every sng_* function is registered ({len(expected)})", found == expected, sorted(set(expected) ^ set(found)))
    upstream = upstream_only(text)
    if update or not SNAPSHOT.exists():
        SNAPSHOT.write_text(upstream, encoding="utf-8", newline="\n")
        print(f"     snapshot written: {SNAPSHOT.relative_to(REPO)} ({len(api_entries(upstream))} entries)")
        return
    old = api_entries(SNAPSHOT.read_text(encoding="utf-8"))
    new = api_entries(upstream)
    added = sorted(set(new) - set(old))
    removed = sorted(set(old) - set(new))
    changed = sorted(name for name in set(old) & set(new) if old[name] != new[name])
    same = not (added or removed or changed)
    check(f"upstream's script API is the snapshot's ({len(new)} entries)", same, "see below")
    for title, names in (("removed", removed), ("changed", changed), ("added", added)):
        for name in names[:40]:
            print(f"     {title}: {name}" + (f"\n       was: {old[name]}\n       now: {new[name]}" if title == "changed" else ""))
        if len(names) > 40:
            print(f"     ... and {len(names) - 40} more {title}")
    if not same:
        print("     If the change is expected: fix what it breaks (layer/native, layer/ext), then --update-snapshot.")


def check_defaults(app: Path, tmp: Path) -> None:
    print("== The layer's defaults: a fresh config gets them, the member's own choice stays", flush=True)
    config = app / "data" / "config.json"  # ArmorPaint keeps its config in its data folder
    check("a fresh copy has no config.json yet", not config.exists())
    report = tmp / "defaults.txt"
    result = run(app, "--background", "--script", os.fspath(script(tmp, "defaults.c", REPORT=os.fspath(report))))
    values = read_report(report)
    check("the defaults script ran", bool(values), result.stdout[-500:])
    check("sng_version says the layer", values.get("version", "").startswith("sng "), values.get("version"))
    check("this start applied the layer's defaults", values.get("defaults") == "applied", values.get("defaults"))
    check("the keymap is the Painter preset", values.get("keymap") == "painter.json", values.get("keymap"))
    check("the theme is the Painter-like theme", values.get("theme") == "painter.json", values.get("theme"))
    saved = json.loads(config.read_text(encoding="utf-8")) if config.is_file() else {}
    check("ArmorPaint wrote them in its config.json", (saved.get("keymap"), saved.get("theme")) == ("painter.json", "painter.json"), saved.get("keymap"))

    # The member chooses ArmorPaint's own keymap and theme: only those two values changed, the rest kept as written.
    text = config.read_text(encoding="utf-8") if config.is_file() else ""
    text = re.sub(r'("keymap"\s*:\s*)"painter\.json"', r'\g<1>"default.json"', text)
    text = re.sub(r'("theme"\s*:\s*)"painter\.json"', r'\g<1>"default.json"', text)
    config.write_text(text, encoding="utf-8")
    report.unlink(missing_ok=True)
    run(app, "--background", "--script", os.fspath(script(tmp, "defaults.c", REPORT=os.fspath(report))))
    values = read_report(report)
    check("a config the member changed is kept", values.get("defaults") == "kept", values.get("defaults"))
    check("their keymap and theme stay", (values.get("keymap"), values.get("theme")) == ("default.json", "default.json"), values)
    saved = json.loads(config.read_text(encoding="utf-8")) if config.is_file() else {}
    check("and stay in config.json after ArmorPaint wrote it again", (saved.get("keymap"), saved.get("theme")) == ("default.json", "default.json"), saved.get("keymap"))


EXPECTED_BINDINGS = {
    "count_start": "1",
    "kind_start": "paint",
    "name_start": "Layer 1",
    "at_out_of_range": "-1",
    "kind_unknown": "",
    "delete_last_refused": "0",
    "select_first": "1",
    "count_before_merge": "2",
    "merged_is_first": "1",
    "count_after_merge": "1",
    "history_after_merge": "Merge Layers",
    "merge_refused": "-1",
    "kind_paint": "paint",
    "current_is_paint": "1",
    "index_paint": "1",
    "history_after_paint": "New Layer",
    "set_name": "1",
    "name_paint": "Rust",
    "history_after_name": "Rename Layer",
    "set_opacity": "1",
    "opacity_paint": "0.500",
    "set_blending": "1",
    "blending_paint": "multiply",
    "set_blending_unknown": "0",
    "set_visible_off": "1",
    "visible_paint_off": "0",
    "undo_visible": "1",
    "visible_paint_undone": "1",
    "redo_count": "1",
    "history_redo_next": "Layer Visibility",
    "redo_visible": "1",
    "visible_paint_redone": "0",
    "kind_fill": "fill",
    "index_fill": "2",
    "history_after_fill": "To Fill Layer",
    "move": "1",
    "index_fill_moved": "1",
    "history_after_move": "Order Layers",
    "undo_move": "1",
    "index_fill_undone": "2",
    "redo_move": "1",
    "index_fill_redone": "1",
    "move_refused": "0",
    "kind_mask_black": "mask",
    "parent_mask_black_is_paint": "1",
    "history_after_mask": "New Black Mask",
    "parent_mask_white_is_fill": "1",
    "mask_refused": "-1",
    "kind_group": "group",
    "parent_paint_is_group": "1",
    "history_after_group": "New Group",
    "group_refused": "-1",
    "kind_dup": "fill",
    "dup_is_new": "1",
    "delete_dup": "1",
    "kind_dup_after_delete": "",
    "history_after_delete": "Delete Layer",
    "bake_set_samples": "1",
    "bake_samples": "64.000",
    "bake_set_ao_radius": "1",
    "bake_ao_radius": "0.250",
    "bake_set_axis": "1",
    "bake_axis": "3.000",
    "bake_set_axis_refused": "0",
    "bake_set_unknown": "0",
    "bake_get_unknown": "-1.000",
    "preset_start": "generic",
    "getenv": "from the smoke test",
    "getenv_unset": "",
    "redraw": "1",
    "mask_invert": "1",
    "history_after_invert": "Invert Mask",
    "mask_invert_refused": "0",
    "mask_apply": "1",
    "kind_white_after_apply": "",
    "history_after_apply": "Apply Mask",
    "export_set_preset": "1",
    "export_set_preset_unknown": "0",
    "export_preset": "singerie",
    "export_set_name": "1",
    "export_name": "smoke",
    "export_set_mode": "1",
    "export_set_mode_unknown": "0",
    "export_set_format": "1",
    "export_set_bits_refused": "0",
    "export_bits": "8",
    "export_textures": "1",
    "done": "yes",
}


def check_bindings(app: Path, tmp: Path) -> None:
    print("== The sng_* functions on a fresh project, then an export with the studio preset", flush=True)
    report = tmp / "bindings.txt"
    exported = tmp / "export"
    exported.mkdir()
    environment = {**os.environ, "SNG_SMOKE": "from the smoke test"}
    result = run(
        app,
        "--background",
        "--script",
        os.fspath(script(tmp, "bindings.c", REPORT=os.fspath(report), EXPORT=os.fspath(exported))),
        environment=environment,
    )
    values = read_report(report)
    if not check("the bindings script ran to its end", values.get("done") == "yes", (result.returncode, result.stdout[-800:], result.stderr[-400:])):
        print("     report so far:\n       " + "\n       ".join(f"{k}={v}" for k, v in values.items()))
    if values:
        for key, expected in EXPECTED_BINDINGS.items():
            check(f"{key} = {expected!r}", values.get(key) == expected, values.get(key))
    print(f"     {values.get('version', '')}; {values.get('history_count', '?')} undo steps available at the end")
    check("the export presets listed include the studio ones", int(values.get("preset_count", "0")) >= 10, values.get("preset_count"))

    preset = json.loads((LAYER / "ext" / "export_presets" / "singerie.json").read_text(encoding="utf-8"))
    for texture in preset["textures"]:
        path = exported / f"smoke_{texture['name']}.png"
        size = png_size(path) if path.is_file() else None
        check(f"exported {path.name} (PNG, 2048 x 2048)", size == (2048, 2048), size)
    extra = sorted(p.name for p in exported.iterdir() if p.name not in {f"smoke_{t['name']}.png" for t in preset["textures"]})
    check("the export wrote nothing else", not extra, extra)


def app_checks(source: Path, update: bool) -> None:
    if not (source / "ArmorPaint.exe").is_file():
        check(f"a build at {source}", False, "build first: python layer/tools/build_windows.py")
        return
    with tempfile.TemporaryDirectory(prefix="sng-smoke-") as folder:
        tmp = Path(folder)
        app = tmp / "armorpaint"
        shutil.copytree(source, app, ignore=shutil.ignore_patterns("config.json"))
        print(f"== The build's folder ({source}), in a copy", flush=True)
        for sub in EXT_FOLDERS:
            for path in sorted((LAYER / "ext" / sub).glob("*")) if (LAYER / "ext" / sub).is_dir() else []:
                laid = app / "data" / sub / path.name
                check(f"data/{sub}/{path.name} is the layer's", laid.is_file() and laid.read_bytes() == path.read_bytes())
        check("ALTERED_VERSION.txt marks the build as altered", (app / "ALTERED_VERSION.txt").is_file())
        check_defaults(app, tmp)  # first: it needs a copy ArmorPaint never started in
        check_api(app, update)
        check_bindings(app, tmp)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--armorpaint", default=os.fspath(DEFAULT_APP), help="the folder holding ArmorPaint.exe")
    parser.add_argument("--static", action="store_true", help="only the checks that need no build")
    parser.add_argument("--update-snapshot", action="store_true", help="rewrite api_snapshot.h from this build")
    args = parser.parse_args()
    static_checks()
    if not args.static:
        app_checks(Path(args.armorpaint).resolve(), args.update_snapshot)
    print(f"\n{'All checks passed.' if not failures else f'{len(failures)} check(s) failed.'}")
    return 0 if not failures else 1


if __name__ == "__main__":
    sys.exit(main())
