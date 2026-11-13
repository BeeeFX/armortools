"""The members' build: a clang build of the layer packed as the archive Singerie's software manager installs
(layer/README.md > Packaging a members' build; Singerie's docs/dev/desktop.md > The software manager).

    python layer/tools/package_windows.py --out <folder>             # the build in ~/.singerie/tools/armorpaint-layer-build
    python layer/tools/package_windows.py --build <app folder> --out <folder>

It refuses a build that isn't one to give members: one made with MSVC (the scratch route), one of another commit than
this repository's HEAD, one made from uncommitted changes. Then it writes, in `--out`:

- `singepaint-<version with - for +>-win64.zip`: one folder holding the app (`ArmorPaint.exe`, `data/`, `license.md`,
  `readme.txt`, `ALTERED_VERSION.txt`, `sng_files.json`): exactly the files the build laid out, as `sng_files.json`
  lists them with their SHA-256. Anything a run of the build added since (a `config.json`, a `singerie/` session
  folder, Singerie's plugin, a keymap a test edited is refused) is left out and said, so a member's first start gets
  the layer's defaults and nothing a test left behind; a listed file that changed or went missing refuses the build;
- `<zip>.sha256` (`<sha256>  <zip>`, as sha256sum writes it) and `<zip>.json`: what an admin enters in Singerie's
  Admin > Desktop app > Builds of our apps (app `armorpaint`, the version, the archive's name, size and SHA-256),
  plus the commits and the toolset.

The version is what the software manager reads (three numbers, `+`, a build part of letters, digits, dots and
dashes; builds ordered by their numbers): `<ArmorPaint's version>+<upstream date>.sng<n>.<upstream>.<layer>`, for
example `1.1.0+2026.10.04.sng16.4c06b93.64a2092`. ArmorPaint's version is its `manifest_version` as three numbers
(`1.1alpha` -> `1.1.0`); the upstream commit's date orders builds of different upstream commits; `n`, the number of
the layer's own commits (`git rev-list --count <pin>..HEAD`), orders builds of the same upstream commit (it only
grows); then the upstream and layer commits, the same as the About box's `<upstream>+sng.<layer>`.

Finally the archive is unpacked with Windows' own tar.exe, as the software manager does, into a temporary folder,
and `ArmorPaint.exe --api` must list the layer's functions there. Nothing is uploaded or published: an admin does
that in Singerie. Standard library only (Python 3.11+). Windows only.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
import tempfile
import tomllib
import zipfile
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
DEFAULT_BUILD = Path.home() / ".singerie" / "tools" / "armorpaint-layer-build" / "armorpaint"
FILES_LIST = "sng_files.json"  # written by build_windows.py in the app folder: every file it laid out, with its hash
TAR = Path(os.environ.get("SystemRoot", r"C:\Windows")) / "System32" / "tar.exe"
VERSION = re.compile(r"^\d+\.\d+\.\d+\+[A-Za-z0-9.-]+$")


class Refused(Exception):
    """The build isn't one to give members."""


def git(*args: str) -> str:
    return subprocess.run(["git", "-C", os.fspath(REPO), *args], capture_output=True, text=True, check=True).stdout.strip()


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with open(path, "rb") as file:
        for chunk in iter(lambda: file.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def three_numbers(manifest_version: str) -> str:
    found = [int(n) for n in re.findall(r"\d+", manifest_version)][:3]
    if not found:
        raise Refused(f"ArmorPaint's version '{manifest_version}' has no numbers.")
    return ".".join(str(n) for n in found + [0] * (3 - len(found)))


def check_build(build: Path) -> tuple[str, str, str]:
    """The build's upstream and layer commits and its toolset, or Refused."""
    if not (build / "ArmorPaint.exe").is_file():
        raise Refused(f"No ArmorPaint.exe in {build}: build first (layer/tools/build_windows.py).")
    note = (build / "ALTERED_VERSION.txt").read_text(encoding="utf-8") if (build / "ALTERED_VERSION.txt").is_file() else ""
    if "Built " not in note:
        raise Refused("The build has no ALTERED_VERSION.txt: it wasn't made by layer/tools/build_windows.py.")
    if "with clang" not in note or "scratch build" in note:
        raise Refused("This build was made with MSVC (the scratch route for tests): members get clang builds only.")
    stamp = json.loads((build / "data" / "version.json").read_text(encoding="utf-8")).get("sha", "")
    found = re.fullmatch(r"([0-9a-f]{7})\+sng\.([0-9a-f]{7})", stamp)
    if not found:
        raise Refused(f"The build's version.json says '{stamp}', not <upstream>+sng.<layer>.")
    upstream, layer = found.groups()
    head = git("rev-parse", "--short=7", "HEAD")
    if layer != head:
        raise Refused(f"The build is of layer commit {layer}, the repository is at {head}: build again.")
    if git("status", "--porcelain", "--untracked-files=no"):
        raise Refused("The repository has uncommitted changes: commit them, build again, then package.")
    toolset = re.search(r"Built [0-9-]+ with (.+?)\.", note)
    laid_out(build)  # refused here when the list is missing or a file changed
    return upstream, layer, toolset.group(1) if toolset else "clang"


def laid_out(build: Path) -> list[str]:
    """The files build_windows.py laid out (`sng_files.json`), each checked unchanged; Refused otherwise."""
    try:
        files = json.loads((build / FILES_LIST).read_text(encoding="utf-8"))["files"]
    except (OSError, ValueError, KeyError):
        raise Refused(f"The build has no {FILES_LIST} (made before the file list): build again.") from None
    changed = [name for name, digest in files.items() if not (build / name).is_file() or sha256(build / name) != digest]
    if changed:
        raise Refused(f"Files of the build changed or went missing since it was made ({', '.join(changed[:5])}): build again.")
    return sorted(files)


def version_of(upstream: str, layer: str) -> str:
    with open(REPO / "layer" / "upstream.toml", "rb") as file:
        pin = tomllib.load(file)["upstream"]
    if not pin["commit"].startswith(upstream):
        raise Refused(f"The build's upstream commit {upstream} isn't the pin {pin['commit'][:7]} (layer/upstream.toml).")
    manifest = re.search(r'manifest_version\s*=\s*"([^"]+)"', (REPO / "paint/sources/globals.h").read_text(encoding="utf-8"))
    main = three_numbers(manifest.group(1) if manifest else "")
    count = int(git("rev-list", "--count", f"{pin['commit']}..HEAD"))
    version = f"{main}+{pin['date'].replace('-', '.')}.sng{count}.{upstream}.{layer}"
    if not VERSION.match(version) or len(version) > 60:
        raise Refused(f"'{version}' isn't a version Singerie's software manager reads.")
    return version


def pack(build: Path, archive: Path, folder: str) -> tuple[int, list[str]]:
    """The files the build laid out (`sng_files.json`, and the list itself) into `archive` under one folder; returns
    how many went in and the other files found in the build folder, left out."""
    names = laid_out(build) + [FILES_LIST]
    found = {path.relative_to(build).as_posix() for path in build.rglob("*") if path.is_file()}
    with zipfile.ZipFile(archive, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as out:
        for name in names:
            out.write(build / name, f"{folder}/{name}")
    return len(names), sorted(found - set(names))


def check_archive(archive: Path, folder: str) -> None:
    """Unpack as the software manager does (Windows' tar.exe) and start the app once (`--api`)."""
    with tempfile.TemporaryDirectory(prefix="sng-package-") as tmp:
        subprocess.run([os.fspath(TAR), "-xf", os.fspath(archive), "-C", tmp], check=True, capture_output=True)
        app = Path(tmp) / folder
        for needed in ("ArmorPaint.exe", "ALTERED_VERSION.txt", "license.md", "data/version.json", "data/licenses/singerie_layer.md"):
            if not (app / needed).is_file():
                raise Refused(f"The archive has no {folder}/{needed}.")
        if (app / "data" / "config.json").exists():
            raise Refused("The archive carries a config.json: a member's first start wouldn't get the defaults.")
        api = subprocess.run([os.fspath(app / "ArmorPaint.exe"), "--api"], cwd=app, capture_output=True, text=True,
                             encoding="utf-8", errors="replace", timeout=180, check=False).stdout
        if "sng_version" not in api:
            raise Refused("The unpacked ArmorPaint doesn't list the sng_* functions in --api.")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--build", default=os.fspath(DEFAULT_BUILD), help="the built app folder (default %(default)s)")
    parser.add_argument("--out", required=True, help="where to write the archive and its .sha256 and .json")
    args = parser.parse_args()
    build, out = Path(args.build).resolve(), Path(args.out).resolve()
    try:
        upstream, layer, toolset = check_build(build)
        version = version_of(upstream, layer)
        name = f"singepaint-{version.replace('+', '-')}"
        out.mkdir(parents=True, exist_ok=True)
        archive = out / f"{name}-win64.zip"
        archive.unlink(missing_ok=True)
        files, left_out = pack(build, archive, name)
        check_archive(archive, name)
    except (Refused, subprocess.CalledProcessError, OSError, ValueError) as exc:
        print(f"Not packaged: {exc}")
        return 1
    digest = sha256(archive)
    size = archive.stat().st_size
    (out / f"{archive.name}.sha256").write_text(f"{digest}  {archive.name}\n", encoding="utf-8", newline="\n")
    record = {
        "app": "armorpaint",
        "label": "Singepaint (based on ArmorPaint)",
        "version": version,
        "file_name": archive.name,
        "size_bytes": size,
        "sha256": digest,
        "upstream_commit": git("rev-parse", upstream),
        "layer_commit": git("rev-parse", "HEAD"),
        "toolset": toolset,
        "files": files,
    }
    (out / f"{archive.name}.json").write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8", newline="\n")
    print(f"Packaged {archive} ({size / 1e6:.1f} MB, {files} files)\n  version {version}\n  SHA-256 {digest}")
    if left_out:
        print(f"  left out, not laid out by the build ({len(left_out)}): " + ", ".join(left_out[:10]))
    print("  unpacked with Windows' tar.exe and started once (--api lists the sng_* functions): ok")
    return 0


if __name__ == "__main__":
    sys.exit(main())
