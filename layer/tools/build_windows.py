"""Build ArmorPaint with this layer on Windows, from a copy of this repository's tree (layer/README.md > Building).

    python layer/tools/build_windows.py                    # into ~/.singerie/tools/armorpaint-layer-build
    python layer/tools/build_windows.py --into D:\\build    # elsewhere
    python layer/tools/build_windows.py --toolset msvc     # force the scratch MSVC route (tests only)

What it does, in order (nothing is ever written inside this repository; every step works in `<into>/src`):

1. notes the repository's state (`git status`), to check at the end that the build changed nothing in it;
2. copies `base`, `paint` (without `paint/build`), `license.md`, `readme.md` and `.clang-format` into `<into>/src`;
3. applies the core patches `layer/patches/series` lists to the copy (`git apply`; one today, restoring the hook
   below, which upstream removed on 22 October 2026), then copies `layer/native/` into
   `<into>/src/paint/plugins/external/`: with that hook, upstream's project compiles that folder in when it exists
   (`paint/plugins/project.c`, `WITH_EXTERNAL`) and calls its `external_init()` from `plugins_init()`; upstream's own
   `.gitignore` lists the folder, so it never shows up as a change to an upstream file. `sng_build.h` there is
   rewritten with the commits being built;
4. runs upstream's build tool there (`base/tools/bin/windows_x64/amake.exe`, without `--compile`): the Visual Studio
   project, the shaders and `build/out/data`;
5. picks the toolset: Visual Studio's ClangCL when it is installed (what upstream and members' builds use); otherwise
   the copy is switched to MSVC (v143) after three mechanical syntax changes MSVC needs, which change nothing
   ArmorPaint does (C23 empty initializers `(void *[]){}` -> `(void *[]){0}`; compound literals initializing
   file-scope variables; the two variable-length arrays of the colour ramp and vector curves nodes). Those changes are
   made in the copy only and never committed: they are not patches, and such a build is for tests only;
6. stamps the version file ArmorPaint's About box and config read (`<upstream>+sng.<layer>`), then builds Release x64
   with MSBuild from Visual Studio's `vcvars64.bat` (found with vswhere);
7. lays the app out in `<into>/armorpaint/`: `ArmorPaint.exe`, `data/`, upstream's `readme.txt` and `license.md`, the
   layer's data files from `layer/ext/` merged into `data/` (keymap and theme presets, export presets, plugins), the
   layer's licence in `data/licenses/`, and `ALTERED_VERSION.txt` saying plainly that this is an altered ArmorPaint
   (zlib licence, point 2);
8. checks the repository's state again: any difference fails the build.

Nothing is downloaded: a missing tool stops the build with what's missing. Logs go to `<into>/logs/`. Windows only.
Standard library only (Python 3.10+).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import time
from datetime import date
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
LAYER = REPO / "layer"
DEFAULT_INTO = Path.home() / ".singerie" / "tools" / "armorpaint-layer-build"
COPIED = ("base", "paint", "license.md", "readme.md", ".clang-format")
VSWHERE = Path(os.environ.get("PROGRAMFILES(X86)", r"C:\Program Files (x86)")) / (
    r"Microsoft Visual Studio\Installer\vswhere.exe"
)
MSVC_TOOLSET = "v143"  # Visual Studio 2022's own C compiler
CLANG_OPTIONS = re.compile(r"-W[\w-]+\s+")  # clang's warning switches, which MSVC's cl.exe doesn't know

# The three mechanical changes MSVC needs (scratch builds only).
EMPTY_INITIALIZER = (re.compile(r"\[\]\)\{\}"), "[]){0}")
FILE_SCOPE_LITERAL = (re.compile(r"^((?:static\s+)?(\w+_t)\s+\w+\s*=\s*)\(\2\)\{", re.MULTILINE), r"\1{")
VARIABLE_LENGTH_ARRAYS = {
    "paint/sources/nodes_material/color_ramp_node.c": (re.compile(r"i32 sorted\[len\];"), "i32 sorted[1024];"),
    "paint/sources/nodes_material/vector_curves_node.c": (re.compile(r"i32 sorted\[num\];"), "i32 sorted[1024];"),
}

# layer/ext/<folder> -> data/<folder> in the laid-out app.
EXT_FOLDERS = ("keymap_presets", "themes", "export_presets", "plugins")

ALTERED_NOTE = """This is Singepaint, an altered version of ArmorPaint (https://github.com/armory3d/armorpaint), based on
ArmorPaint. Its title bar says "Singepaint (based on ArmorPaint)", and its About box shows the commit as
{upstream}+sng.{layer}.

It is ArmorPaint at upstream commit {upstream} with the Singerie layer (commit {layer}) added: Painter-like default
shortcuts and theme, studio export presets and the sng_* script functions (FORK.md and layer/README.md in the
layer's repository). ArmorPaint is zlib-licensed (license.md, kept here); the layer's own files are zlib-licensed
too (data/licenses/singerie_layer.md).

Built {built} with {toolset}.{scratch}
"""

SCRATCH_NOTE = """
This build used MSVC instead of clang, after three mechanical syntax changes made in a copy of the sources (C23
empty initializers, file-scope compound literals, two variable-length arrays): it is a scratch build for tests,
not one to give to members."""


class Missing(Exception):
    """A tool the build needs isn't on this PC (nothing is ever downloaded)."""


def git(*args: str) -> str:
    """Run git in this repository (never anywhere else) and return its output."""
    return subprocess.run(
        ["git", "-C", os.fspath(REPO), *args], capture_output=True, text=True, check=True
    ).stdout.strip()


def read_pin() -> dict[str, str]:
    import tomllib

    with open(LAYER / "upstream.toml", "rb") as file:
        return tomllib.load(file)["upstream"]


def copy_tree(target: Path) -> None:
    if target.exists():
        shutil.rmtree(target)
    target.mkdir(parents=True)
    for name in COPIED:
        source = REPO / name
        if source.is_dir():
            # paint/build is a local build's output; paint/plugins/external is laid out from layer/native below.
            ignore = shutil.ignore_patterns("build", "external") if name == "paint" else None
            shutil.copytree(source, target / name, ignore=ignore)
        elif source.is_file():
            shutil.copy2(source, target / name)
    for needed in ("base", "paint"):
        if not (target / needed).is_dir():
            raise Missing(f"{REPO} has no {needed} folder: is it ArmorPaint's repository?")


def apply_patches(src: Path, environment: dict) -> list[str]:
    """layer/patches/series, in order, applied to the copy in `src` (never to the repository): layer/patches/README.md."""
    names = [
        line.strip()
        for line in (LAYER / "patches" / "series").read_text(encoding="utf-8").splitlines()
        if line.strip() and not line.lstrip().startswith("#")
    ]
    for name in names:
        patch_file = LAYER / "patches" / name
        if not patch_file.is_file():
            raise Missing(f"layer/patches/series lists {name}, which isn't in layer/patches/.")
        result = subprocess.run(
            ["git", "apply", "--whitespace=nowarn", os.fspath(patch_file)],
            cwd=src,
            env=environment,
            capture_output=True,
            text=True,
        )
        if result.returncode != 0:
            raise RuntimeError(
                f"The core patch {name} no longer applies to upstream (refresh it, or drop it if upstream took it):\n"
                + result.stderr.strip()
            )
    return names


def lay_out_native(src: Path, upstream: str, layer: str) -> None:
    """layer/native -> paint/plugins/external, with sng_build.h saying what is being built."""
    external = src / "paint" / "plugins" / "external"
    shutil.copytree(LAYER / "native", external)
    (external / "sng_build.h").write_text(
        "// Written by layer/tools/build_windows.py for this build.\n"
        "#pragma once\n"
        f'#define SNG_UPSTREAM_COMMIT "{upstream}"\n'
        f'#define SNG_LAYER_COMMIT "{layer}"\n'
        f'#define SNG_BUILD_DATE "{date.today().isoformat()}"\n',
        encoding="utf-8",
    )


INTERNALS_BEGIN = "// BEGIN internals not in upstream's headers"
INTERNALS_END = "// END internals not in upstream's headers"


def check_internals(src: Path) -> int:
    """The bindings file declares a few of ArmorPaint's globals and functions itself (upstream defines them without a
    header). A C declaration that no longer matches its definition still links, so before building, each one must
    match a definition in upstream's sources word for word (spaces aside). Returns how many were checked."""
    text = (LAYER / "native" / "sng_bindings.c").read_text(encoding="utf-8")
    block = text.split(INTERNALS_BEGIN, 1)[1].split(INTERNALS_END, 1)[0]
    wanted = [line.strip() for line in block.splitlines() if line.strip() and not line.strip().startswith("//")]
    definitions = set()
    for path in list((src / "paint" / "sources").rglob("*.c")) + [src / "paint" / "sources" / "globals.h"]:
        for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
            if line and not line[0].isspace():
                definitions.add(re.sub(r"\s+", "", line))
    missing = []
    for declaration in wanted:
        key = re.sub(r"\s+", "", declaration.removeprefix("extern ")).removesuffix(";")
        if declaration.startswith("extern "):
            found = any(d.startswith(key) and d[len(key) : len(key) + 1] in ("=", ";") for d in definitions)
        else:
            found = key + "{" in definitions
        if not found:
            missing.append(declaration)
    if missing:
        raise RuntimeError(
            "layer/native/sng_bindings.c declares internals upstream no longer defines that way (fix them there):\n  "
            + "\n  ".join(missing)
        )
    return len(wanted)


def patch(path: Path, pattern: re.Pattern, replacement: str) -> int:
    text = path.read_text(encoding="utf-8", errors="surrogateescape")
    new, count = pattern.subn(replacement, text)
    if count:
        path.write_text(new, encoding="utf-8", errors="surrogateescape", newline="")
    return count


def apply_msvc_changes(src: Path) -> dict[str, int]:
    """The three mechanical changes, in the copy; how many places each changed. The layer's own code needs none."""
    code = [
        path
        for folder in ("base/sources", "paint/sources", "paint/plugins")
        for pattern in ("*.c", "*.h")
        for path in (src / folder).rglob(pattern)
    ]
    external = src / "paint" / "plugins" / "external"
    for path in external.rglob("*.[ch]"):
        text = path.read_text(encoding="utf-8")
        if EMPTY_INITIALIZER[0].search(text) or FILE_SCOPE_LITERAL[0].search(text):
            raise RuntimeError(f"{path.name}: the layer's own code must build with MSVC unchanged (portable C only).")
    return {
        "empty initializers": sum(patch(path, *EMPTY_INITIALIZER) for path in code),
        "file-scope compound literals": sum(patch(path, *FILE_SCOPE_LITERAL) for path in code),
        "variable-length arrays": sum(patch(src / name, *change) for name, change in VARIABLE_LENGTH_ARRAYS.items()),
    }


def switch_to_msvc(project: Path) -> None:
    text = project.read_text(encoding="utf-8")
    text = text.replace("<PlatformToolset>ClangCL</PlatformToolset>", f"<PlatformToolset>{MSVC_TOOLSET}</PlatformToolset>")
    text = re.sub(
        r"<AdditionalOptions>(.*?)</AdditionalOptions>",
        lambda m: f"<AdditionalOptions>{CLANG_OPTIONS.sub('', m.group(1))}</AdditionalOptions>",
        text,
    )
    project.write_text(text, encoding="utf-8")


def find_visual_studio() -> tuple[Path, bool]:
    """Visual Studio's vcvars64.bat, and whether its ClangCL toolset (with clang-cl) is installed."""
    if not VSWHERE.is_file():
        raise Missing(f"vswhere isn't on this PC ({VSWHERE}): install Visual Studio 2022 with its C++ tools.")
    found = subprocess.run(
        [os.fspath(VSWHERE), "-products", "*", "-latest", "-find", r"VC\Auxiliary\Build\vcvars64.bat"],
        capture_output=True,
        text=True,
        check=False,
    ).stdout.strip()
    if not found or not Path(found.splitlines()[0]).is_file():
        raise Missing("Visual Studio's C++ tools (vcvars64.bat) aren't on this PC: add 'Desktop development with C++'.")
    vcvars = Path(found.splitlines()[0])
    root = vcvars.parents[3]
    toolsets = root / "MSBuild" / "Microsoft" / "VC" / "v170" / "Platforms" / "x64" / "PlatformToolsets"
    has_clang = (toolsets / "ClangCL").is_dir() and (root / "VC" / "Tools" / "Llvm" / "x64" / "bin" / "clang-cl.exe").is_file()
    if not has_clang and not (toolsets / MSVC_TOOLSET).is_dir():
        raise Missing(f"Neither Visual Studio's ClangCL nor its {MSVC_TOOLSET} toolset is on this PC ({toolsets}).")
    return vcvars, has_clang


def run_logged(command: list[str], cwd: Path, log: Path, what: str, environment: dict | None = None) -> None:
    started = time.monotonic()
    with open(log, "w", encoding="utf-8", errors="replace") as out:
        status = subprocess.run(command, cwd=cwd, env=environment, stdout=out, stderr=subprocess.STDOUT, check=False).returncode
    print(f"{what}: exit {status} in {time.monotonic() - started:.0f} s (log: {log})", flush=True)
    if status != 0:
        tail = log.read_text(encoding="utf-8", errors="replace").splitlines()[-30:]
        raise RuntimeError(f"{what} failed:\n" + "\n".join(tail))


def stamp_version(src: Path, sha: str) -> None:
    """The commit in the version file amake wrote (ArmorPaint's About box shows it, and a config.json written by
    another build is discarded): amake's own `git log` found nothing in the copy. The date stays amake's."""
    for path in (src / "paint" / "build" / "version.json", src / "paint" / "build" / "out" / "data" / "version.json"):
        try:
            data = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, ValueError):
            continue
        path.write_text(f'{{ "sha": "{sha}", "date": "{data.get("date", "")}" }}', encoding="utf-8")


def lay_out_app(src: Path, app: Path, note: str) -> list[str]:
    """The app folder: upstream's output plus the layer's data files. Returns the layer files added."""
    if app.exists():
        shutil.rmtree(app)
    out = src / "paint" / "build" / "out"
    shutil.copytree(out / "data", app / "data")
    shutil.copy2(src / "paint" / "build" / "x64" / "Release" / "ArmorPaint.exe", app / "ArmorPaint.exe")
    for name in ("readme.txt",):
        if (out / name).is_file():
            shutil.copy2(out / name, app / name)
    shutil.copy2(src / "license.md", app / "license.md")
    added = []
    for folder in EXT_FOLDERS:
        source = LAYER / "ext" / folder
        if not source.is_dir():
            continue
        (app / "data" / folder).mkdir(parents=True, exist_ok=True)
        for path in sorted(source.iterdir()):
            if path.is_file() and not path.name.startswith(("README", ".")):
                if (app / "data" / folder / path.name).exists():
                    raise RuntimeError(f"layer/ext/{folder}/{path.name} would replace upstream's own file: rename it.")
                shutil.copy2(path, app / "data" / folder / path.name)
                added.append(f"data/{folder}/{path.name}")
    (app / "data" / "licenses").mkdir(parents=True, exist_ok=True)
    shutil.copy2(LAYER / "LICENSE.md", app / "data" / "licenses" / "singerie_layer.md")
    (app / "ALTERED_VERSION.txt").write_text(note, encoding="utf-8")
    return added


def build(into: Path, toolset: str) -> int:
    into.mkdir(parents=True, exist_ok=True)
    logs = into / "logs"
    logs.mkdir(exist_ok=True)
    pin = read_pin()
    upstream = pin["commit"][:7]
    layer = git("rev-parse", "--short=7", "HEAD")
    dirty = git("status", "--porcelain")
    if dirty:
        print("Note: the repository has uncommitted changes; the build includes them.", flush=True)
    if subprocess.run(["git", "-C", os.fspath(REPO), "merge-base", "--is-ancestor", pin["commit"], "HEAD"]).returncode != 0:
        print(f"Note: HEAD doesn't contain the pinned upstream commit {upstream} (layer/upstream.toml).", flush=True)
    vcvars, has_clang = find_visual_studio()
    use_clang = has_clang if toolset == "auto" else toolset == "clang"
    if use_clang and not has_clang:
        raise Missing("Visual Studio's ClangCL toolset isn't installed (Individual components > C++ Clang tools).")
    print(f"Repository: {REPO} (layer {layer}, upstream {upstream})\nBuilding in: {into}", flush=True)

    src = into / "src"
    copy_tree(src)
    print(f"ok   the bindings' {check_internals(src)} hand-declared internals match upstream's definitions.", flush=True)
    environment = {**os.environ, "GIT_CEILING_DIRECTORIES": os.fspath(into)}
    patches = apply_patches(src, environment)
    print(f"ok   {len(patches)} core patch(es) applied to the copy: {', '.join(patches) or 'none'}.", flush=True)
    lay_out_native(src, upstream, layer)
    amake = src / "base" / "tools" / "bin" / "windows_x64" / "amake.exe"
    if not amake.is_file():
        raise Missing(f"Upstream's build tool isn't in the tree ({amake.relative_to(src)}).")
    run_logged([os.fspath(amake)], src / "paint", logs / "amake.log", "amake (project, shaders, data)", environment)
    project = src / "paint" / "build" / "ArmorPaint.vcxproj"
    if "external" not in project.read_text(encoding="utf-8"):
        raise RuntimeError("The Visual Studio project doesn't list paint/plugins/external: upstream's hook moved?")
    if use_clang:
        toolset_text, scratch = "clang (Visual Studio's ClangCL)", ""
    else:
        switch_to_msvc(project)
        counts = apply_msvc_changes(src)
        print("MSVC changes in the copy: " + ", ".join(f"{name} {count}" for name, count in counts.items()), flush=True)
        toolset_text, scratch = f"MSVC ({MSVC_TOOLSET})", SCRATCH_NOTE
    stamp_version(src, f"{upstream}+sng.{layer}")
    script = src / "paint" / "build" / "layer_build.bat"
    script.write_text(
        f'@call "{vcvars}"\r\n@MSBuild.exe "{project}" /m /clp:ErrorsOnly /p:Configuration=Release,Platform=x64\r\n',
        encoding="utf-8",
    )
    run_logged(["cmd", "/c", os.fspath(script)], script.parent, logs / "msbuild.log", f"MSBuild ({toolset_text}, Release x64)")

    note = ALTERED_NOTE.format(upstream=upstream, layer=layer, built=date.today().isoformat(), toolset=toolset_text, scratch=scratch)
    added = lay_out_app(src, into / "armorpaint", note)
    print(f"ArmorPaint with the layer laid out in {into / 'armorpaint'} ({len(added)} layer data files)", flush=True)

    if git("status", "--porcelain") != dirty:
        print("FAIL the build changed files in the repository:\n" + git("status", "--porcelain"))
        return 1
    print("ok   the repository is as it was before the build.")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--into", default=os.fspath(DEFAULT_INTO), help="where to build (default %(default)s)")
    parser.add_argument("--toolset", choices=("auto", "clang", "msvc"), default="auto", help="default: clang when installed")
    args = parser.parse_args()
    into = Path(args.into).resolve()
    if into == REPO or REPO in into.parents:
        print("Build outside the repository: the build never writes in it.")
        return 2
    try:
        return build(into, args.toolset)
    except Missing as exc:
        print(f"Missing: {exc}")
        return 3
    except (RuntimeError, subprocess.CalledProcessError) as exc:
        print(exc)
        return 1


if __name__ == "__main__":
    sys.exit(main())
