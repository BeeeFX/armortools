"""The layer's smoke test (layer/README.md > Testing): the layer's files checked against this tree's upstream sources,
then the built ArmorPaint run headless.

    python layer/tests/smoke/smoke.py                          # the build in ~/.singerie/tools/armorpaint-layer-build
    python layer/tests/smoke/smoke.py --armorpaint <folder>    # another build (the folder holding ArmorPaint.exe)
    python layer/tests/smoke/smoke.py --static                 # only the checks that need no build
    python layer/tests/smoke/smoke.py --update-snapshot        # after reviewing an upstream API change
    python layer/tests/smoke/smoke.py --allow-additions        # new upstream API entries pass (layer/tools/watch.py)

Without a build (checked against upstream's sources in this tree):
- the theme follows ArmorPaint's theme fields in their order (its theme reader is positional, not by name);
- the keymap preset only names actions ArmorPaint has (an upstream rename would silently drop a shortcut), and gives
  no shortcut, modifiers included, to two actions (ArmorPaint tells `1` from `Ctrl+1`, and would fire both of a pair);
- `upstream.toml`'s `amake_sha256` is the hash of the build tool in this tree (the build runs it: layer/README.md >
  Keeping up with upstream), and the build refuses another one without `--accept-tools`;
- packaging takes only the files the build laid out (`sng_files.json`): one a test run added is left out and said, one
  that changed refuses the package;
- the export presets have upstream's shape, only channel ids ArmorPaint exports, and names that say their colour
  space as Singerie's rule wants (`basecolor_srgb`, `roughness_raw`...); no layer file replaces an upstream one.

With the build (each run in a copy of its folder, so the build stays untouched):
- the layer's data files are in the app's data folder;
- `--api`: every sng_* function registered, and ArmorPaint's own script API (functions, structs, enums, globals and
  the material nodes) compared with `api_snapshot.h`, so an upstream API change is caught;
- the first start of a fresh copy gives it the Painter keymap and theme, written by ArmorPaint in its config.json,
  and its title bar says "Singepaint (based on ArmorPaint)" (also once a change marks the project unsaved);
  a config with the member's own choice (ArmorPaint's default keymap and theme) keeps it; and a fresh config with a
  `data/sng_keep.json` (the preferences Singerie's launcher carries over from the build before) takes the names it
  knows and leaves out the rest;
- `scripts/bindings.c`: every sng_* function on a fresh project, then exports: the studio preset (names, PNG, size,
  and what the pictures hold: the packed `orm_raw` must be the three single maps, height flat at 0), the packed preset,
  and 16 bits (EXR).

Exit status 0 when everything passed. Standard library only (Python 3.11+).
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
SHOWN_NAME = "Singepaint (based on ArmorPaint)"  # in the title bar instead of "ArmorPaint" (Baptiste, 5 October 2026)

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
        clashes = {k: v for k, v in shortcuts(merged).items() if len(v) > 1 and frozenset(v) not in SHARED}
        check(f"keymap_presets/{path.name} gives no shortcut (modifiers included) to two actions", not clashes, clashes)
        # Keys upstream reads itself in a tab (F2 renames the layer or mesh under the mouse, F5 refreshes the browser):
        # a preset action on one of them would fire as well (the F2 clash with the node editor, 1 November 2026).
        taken = {key for key, actions in plain.items() if key in hard_wired_keys()}
        check(f"keymap_presets/{path.name} leaves upstream's own keys alone ({', '.join(sorted(hard_wired_keys()))})",
              not taken, {key: plain[key] for key in taken})

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

    # The build tool: the one upstream.toml names, by its hash (build_windows.py refuses another without --accept-tools).
    pinned, actual = amake_hashes()
    check("upstream.toml's amake_sha256 is the hash of this tree's build tool", bool(pinned) and pinned == actual, (pinned, actual))
    tool_checks()


def tool(name: str):
    """layer/tools/<name>.py as a module (the build and packaging tools, checked without building)."""
    import importlib.util

    spec = importlib.util.spec_from_file_location(f"sng_{name}", LAYER / "tools" / f"{name}.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def tool_checks() -> None:
    """The build tool's pin and the package's file list, on stand-in files (no build needed)."""
    import zipfile

    build_windows, package_windows = tool("build_windows"), tool("package_windows")
    with tempfile.TemporaryDirectory(prefix="sng-static-") as folder:
        tmp = Path(folder)
        other = tmp / "amake.exe"
        other.write_bytes(b"MZ not upstream's pinned build tool")
        try:
            build_windows.check_amake(other, accept_tools=False)
            refused = ""
        except RuntimeError as exc:
            refused = str(exc)
        check("the build refuses a build tool upstream.toml doesn't pin, naming --accept-tools", "--accept-tools" in refused, refused)
        check("--accept-tools lets a person run it once they read what changed", bool(build_windows.check_amake(other, accept_tools=True)))
        pinned = REPO / build_windows.AMAKE
        if pinned.is_file():
            check("the tree's own build tool passes", build_windows.check_amake(pinned, accept_tools=False) == amake_hashes()[0])

        app = tmp / "armorpaint"
        (app / "data" / "plugins").mkdir(parents=True)
        (app / "ArmorPaint.exe").write_bytes(b"MZ")
        (app / "data" / "version.json").write_text('{"sha": "0000000+sng.0000000"}', encoding="utf-8")
        count = build_windows.write_files_list(app)
        (app / "data" / "plugins" / "singerie.c").write_text("void main() {}\n", encoding="utf-8")  # a test run's
        (app / "data" / "config.json").write_text("{}", encoding="utf-8")
        archive = tmp / "out.zip"
        files, left_out = package_windows.pack(app, archive, "singepaint")
        names = sorted(zipfile.ZipFile(archive).namelist())
        check(f"the package holds the {count} files the build laid out and their list", files == count + 1 and names == [
            "singepaint/ArmorPaint.exe", "singepaint/data/version.json", f"singepaint/{build_windows.FILES_LIST}"], names)
        check("a file a test run left in the build is left out, and said", left_out == ["data/config.json", "data/plugins/singerie.c"], left_out)
        (app / "data" / "version.json").write_text('{"sha": "changed"}', encoding="utf-8")
        try:
            package_windows.laid_out(app)
            refused = ""
        except package_windows.Refused as exc:
            refused = str(exc)
        check("a laid-out file that changed since the build refuses the package, named", "data/version.json" in refused, refused)


# Pairs upstream's own default keymap gives one shortcut on purpose: they fire in different places.
SHARED = {
    frozenset({"node_overview", "brush_camera_align"}),
    frozenset({"node_search", "keymap_search"}),
    frozenset({"action_pan", "view_pivot_center"}),
}


def shortcuts(keymap: dict[str, str]) -> dict[tuple[str, ...], list[str]]:
    """The actions of each whole shortcut (its modifiers and its key: ArmorPaint fires a shortcut only when exactly its
    modifiers are held, keymap_shortcut in paint/sources/keymap.c), leaving out a bare modifier or mouse button (the
    paint stroke, the decal mask: read where they apply)."""
    found: dict[tuple[str, ...], list[str]] = {}
    for action, key in keymap.items():
        if not key:
            continue
        *modifiers, last = key.lower().split("+")
        if last in ("ctrl", "shift", "alt") or (not modifiers and last in ("left", "right", "middle", "number")):
            continue
        found.setdefault((*sorted(modifiers), last), []).append(action)
    return found


def amake_hashes() -> tuple[str, str]:
    """upstream.toml's amake_sha256 and the hash of the amake.exe in this tree ("" when either is missing)."""
    import hashlib

    found = re.search(r'(?m)^amake_sha256\s*=\s*"([0-9a-f]*)"', (LAYER / "upstream.toml").read_text(encoding="utf-8"))
    tool = REPO / "base" / "tools" / "bin" / "windows_x64" / "amake.exe"
    actual = hashlib.sha256(tool.read_bytes()).hexdigest() if tool.is_file() else ""
    return (found.group(1) if found else ""), actual


def hard_wired_keys() -> set[str]:
    """The function keys ArmorPaint's own panels read directly (`key_code == KEY_CODE_F2`, not through the keymap), as
    keymap names ("f2")."""
    keys = set()
    pattern = re.compile(r"key_code\s*==\s*KEY_CODE_F(\d+)\b")
    for path in (REPO / "paint" / "sources").rglob("*.c"):
        keys |= {f"f{n}" for n in pattern.findall(path.read_text(encoding="utf-8", errors="replace"))}
    return keys


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
    """One ArmorPaint run; one that doesn't quit within TIMEOUT is stopped and fails a check (the checks after it
    then say what its report lacks)."""
    command = [os.fspath(app / "ArmorPaint.exe"), *arguments]
    try:
        return subprocess.run(
            command,
            cwd=app,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=TIMEOUT,
            env=environment,
            check=False,
        )
    except subprocess.TimeoutExpired:  # subprocess.run has stopped that ArmorPaint (only it)
        check(f"ArmorPaint {' '.join(arguments[:2])} quit within {TIMEOUT} s", False, "stopped")
        return subprocess.CompletedProcess(command, -1, "", "timed out")


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


def png_pixel(path: Path, x: int, y: int = 0) -> tuple[int, ...] | None:
    """The 8-bit pixel at (x, y) of a PNG, decoded with the standard library (rows 0 to y unfiltered: filter types 0
    to 4); None for a file that isn't an 8-bit, non-interlaced PNG."""
    import zlib

    data = path.read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        return None
    at, idat, header = 8, b"", None
    while at + 8 <= len(data):
        length, kind = struct.unpack(">I4s", data[at : at + 8])
        body = data[at + 8 : at + 8 + length]
        if kind == b"IHDR":
            header = struct.unpack(">IIBBBBB", body)
        elif kind == b"IDAT":
            idat += body
        at += 12 + length
    if header is None:
        return None
    width, height, depth, colour, _, _, interlace = header
    channels = {0: 1, 2: 3, 4: 2, 6: 4}.get(colour)
    if depth != 8 or channels is None or interlace or not (0 <= x < width and 0 <= y < height):
        return None
    raw = zlib.decompress(idat)
    stride = width * channels
    previous = bytearray(stride)
    for row in range(y + 1):
        kind = raw[row * (stride + 1)]
        line = bytearray(raw[row * (stride + 1) + 1 : (row + 1) * (stride + 1)])
        for i in range(stride):
            left = line[i - channels] if i >= channels else 0
            up = previous[i]
            corner = previous[i - channels] if i >= channels else 0
            if kind == 1:
                line[i] = (line[i] + left) & 0xFF
            elif kind == 2:
                line[i] = (line[i] + up) & 0xFF
            elif kind == 3:
                line[i] = (line[i] + (left + up) // 2) & 0xFF
            elif kind == 4:
                guess = left + up - corner
                pa, pb, pc = abs(guess - left), abs(guess - up), abs(guess - corner)
                line[i] = (line[i] + (left if pa <= pb and pa <= pc else up if pb <= pc else corner)) & 0xFF
        previous = line
    return tuple(previous[x * channels : (x + 1) * channels])


def check_api(app: Path, update: bool, allow_additions: bool = False, snapshot_out: Path | None = None) -> None:
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
    if snapshot_out:  # the watch keeps this build's API, to become the snapshot once everything passed
        snapshot_out.write_text(upstream, encoding="utf-8", newline="\n")
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
    if allow_additions and same is False and not (removed or changed):
        # New upstream functions can't break the layer or a plugin; a removed or changed one can.
        check(f"upstream's script API keeps every snapshot entry ({len(added)} new, none removed or changed)", True)
        same = True
    else:
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
    check(f"the build's shown name is {SHOWN_NAME!r}", values.get("name") == SHOWN_NAME, values.get("name"))
    check("the title bar shows it from the start", values.get("title") == f"untitled - {SHOWN_NAME}", values.get("title"))
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

    # An update: no config.json in the new version's folder, and the preferences of the build before carried over by
    # name in data/sng_keep.json (Singerie's launcher writes it). Known names that fit are taken, over the layer's
    # defaults; a preset this build lacks, a value Preferences wouldn't allow and an unknown name are left out.
    config.unlink(missing_ok=True)
    keep = {
        "keymap": "default.json",
        "theme": "not_in_this_build.json",
        "window_scale": 1.5,
        "undo_steps": 12,
        "pressure_radius": False,
        "pressure_sensitivity": 99,
        "layer_res": 4,
    }
    (app / "data" / "sng_keep.json").write_text(json.dumps(keep), encoding="utf-8")
    report.unlink(missing_ok=True)
    run(app, "--background", "--script", os.fspath(script(tmp, "defaults.c", REPORT=os.fspath(report))))
    values = read_report(report)
    check("a kept keymap name is applied on a fresh config", values.get("keymap") == "default.json", values.get("keymap"))
    check("a kept theme this build lacks leaves the layer's", values.get("theme") == "painter.json", values.get("theme"))
    check("four preferences carried over (keymap, UI scale, undo steps, pen pressure on size)", values.get("carried") == "4", values.get("carried"))
    saved = json.loads(config.read_text(encoding="utf-8")) if config.is_file() else {}
    kept = (saved.get("window_scale"), saved.get("undo_steps"), saved.get("pressure_radius"), saved.get("pressure_sensitivity") != 99)
    check("ArmorPaint saved them in its config.json, the out-of-range one left out", kept == (1.5, 12, False, True), kept)
    (app / "data" / "sng_keep.json").unlink(missing_ok=True)


EXPECTED_BINDINGS = {
    "title_start": f"untitled - {SHOWN_NAME}",
    "title_after_change": f"untitled* - {SHOWN_NAME}",
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
    "set_opacity_nan_refused": "0",
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
    "bake_set_samples_refused": "0",
    "bake_set_high_poly": "1",
    "bake_set_high_poly_refused": "0",
    "bake_set_high_poly_negative": "0",
    "asset_repoint_unknown": "0",
    "mesh_reimport_missing": "0",
    "export_mode_start": "visible",
    "export_format_start": "png",
    "export_destination_start": "disk",
    "export_mode_selected": "selected",
    "info_export_mode": "selected",
    "info_export_format": "jpg",
    "export_set_destination": "1",
    "export_destination_project": "project",
    "info_export_destination": "project",
    "export_set_destination_unknown": "0",
    "info_unknown": "",
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
    "export_textures_missing_folder": "0",
    "export_textures": "1",
    "export_set_packed": "1",
    "export_textures_packed": "1",
    "export_set_bits_16": "1",
    "export_textures_16": "1",
    "done": "yes",
}


def check_bindings(app: Path, tmp: Path) -> None:
    print("== The sng_* functions on a fresh project, then an export with the studio preset", flush=True)
    report = tmp / "bindings.txt"
    exported, packed, deep = tmp / "export", tmp / "export_packed", tmp / "export_16"
    for folder in (exported, packed, deep):
        folder.mkdir()
    environment = {**os.environ, "SNG_SMOKE": "from the smoke test"}
    filled = script(
        tmp,
        "bindings.c",
        REPORT=os.fspath(report),
        EXPORT=os.fspath(exported),
        EXPORT_PACKED=os.fspath(packed),
        EXPORT_16=os.fspath(deep),
        MISSING=os.fspath(tmp / "not_there" / "nothing.png"),
    )
    result = run(app, "--background", "--script", os.fspath(filled), environment=environment)
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
    check_pictures(exported, packed, deep, preset)


def check_pictures(exported: Path, packed: Path, deep: Path, preset: dict) -> None:
    """What the maps hold, from one pixel each (the fill layer on top covers the mesh, so every pixel of a map is the
    same): each channel must be where the presets say, which a swapped channel in a preset or a change of a channel's
    meaning upstream would break."""
    centre = 1024

    def pixel(folder: Path, name: str) -> tuple[int, ...] | None:
        path = folder / f"smoke_{name}.png"
        return png_pixel(path, centre) if path.is_file() else None

    occlusion, roughness, metallic = (pixel(exported, f"{n}_raw") for n in ("occlusion", "roughness", "metallic"))
    orm = pixel(packed, "orm_raw")
    single = tuple(p[0] if p else None for p in (occlusion, roughness, metallic))
    check("the packed orm_raw holds occlusion, roughness, metallic in red, green, blue", orm is not None and orm[:3] == single, (orm, single))
    check("roughness and metallic differ here, so a swap of the two would show", single[1] != single[2], single)
    base, base_packed = pixel(exported, "basecolor_srgb"), pixel(packed, "basecolor_srgb")
    check("both presets write the same base colour", base is not None and base_packed is not None and base[:3] == base_packed[:3], (base, base_packed))
    height = pixel(exported, "height_raw")
    check("an unpainted height map is 0 (ArmorPaint's flat: Singerie builds the material with Midlevel 0)", height is not None and height[0] == 0, height)
    expected = {f"smoke_{t['name']}.exr" for t in preset["textures"]}
    found = {p.name for p in deep.iterdir()}
    check("at 16 bits the studio preset writes EXR", found == expected, sorted(found ^ expected))


def app_checks(source: Path, update: bool, allow_additions: bool = False, snapshot_out: Path | None = None) -> None:
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
        check_api(app, update, allow_additions, snapshot_out)
        check_bindings(app, tmp)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--armorpaint", default=os.fspath(DEFAULT_APP), help="the folder holding ArmorPaint.exe")
    parser.add_argument("--static", action="store_true", help="only the checks that need no build")
    parser.add_argument("--update-snapshot", action="store_true", help="rewrite api_snapshot.h from this build")
    parser.add_argument("--allow-additions", action="store_true", help="new upstream API entries pass (the watch)")
    parser.add_argument("--snapshot-out", help="also write this build's upstream API to that file (the watch)")
    args = parser.parse_args()
    static_checks()
    if not args.static:
        out = Path(args.snapshot_out).resolve() if args.snapshot_out else None
        app_checks(Path(args.armorpaint).resolve(), args.update_snapshot, args.allow_additions, out)
    print(f"\n{'All checks passed.' if not failures else f'{len(failures)} check(s) failed.'}")
    return 0 if not failures else 1


if __name__ == "__main__":
    sys.exit(main())
