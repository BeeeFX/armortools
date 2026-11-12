"""The weekly upstream watch, run from a PC (GitHub Actions are off on this repository): does the layer still build and
pass its smoke test on upstream ArmorPaint's newest `main`? When it does, follow it (layer/README.md > Keeping up).

    python layer/tools/watch.py                          # check only: nothing in the repository changes
    python layer/tools/watch.py --singerie <repo>        # also run Singerie's pipeline plugin test on the build
    python layer/tools/watch.py --apply                  # follow upstream when everything passed
    python layer/tools/watch.py --apply --push           # and push main and sng/main to origin (BeeeFX/armortools)
    python layer/tools/watch.py --accept-tools ...       # also when upstream changed the programs in its tree

What it does:

1. fetches upstream (`git fetch upstream main`) and compares its newest commit with the pin (layer/upstream.toml):
   nothing to do when they're the same; lists every program or library upstream changed since the pin
   (`base/tools/bin`, `*.exe`, `*.dll`, `*.lib`): the build runs upstream's own build tool, so when it changed, the
   build refuses it and the watch stops there unless `--accept-tools` is given (after reading what changed);
2. in a temporary git worktree, takes upstream's newest commit and puts this branch's `layer/` and `FORK.md` in it
   (the layer edits no upstream file, so that is exactly what merging would give);
3. builds there (layer/tools/build_windows.py, into `--into`, by default ~/.singerie/tools/armorpaint-layer-watch);
4. runs the smoke test on that build with `--allow-additions`: upstream functions, structs or nodes that are new
   pass (they can't break the layer or a plugin); removed or changed ones fail and are listed;
5. with `--singerie <Singerie's repository>`, runs Singerie's pipeline plugin test on the build
   (`plugins/armorpaint/tests/run_tests.py --armorpaint <build>`);
6. writes a report (`<into>/watch-report.md`): the upstream commits since the pin, each step's result, the API's
   new entries;
7. with `--apply`, and only when every step passed: fast-forwards `main` to upstream's commit, merges `main` into
   `sng/main` (the branch must be checked out, with no uncommitted changes), sets the pin to the new commit (and the
   build tool's hash), writes the new API snapshot, and commits. `--push` then pushes `main` and `sng/main` (never
   forced). `--apply` is a person's step, after reading the report: the scheduled run checks only.

When a step fails, nothing changes in the repository: the report says what broke (a compile error points at
layer/native/sng_bindings.c, a static check at a data file, an API difference names the entries), to fix by hand.
`--trailer "Key: value"` adds a line to the commits it makes. Exit status: 0 when up to date or passing, 1 when a
step failed, 2 when it can't run. Windows only (the build). Standard library only (Python 3.11+).
"""

from __future__ import annotations

import argparse
import hashlib
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time
import tomllib
from datetime import date
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
LAYER = REPO / "layer"
PIN = LAYER / "upstream.toml"
SNAPSHOT = LAYER / "tests" / "smoke" / "api_snapshot.h"
BRANCH = "sng/main"
OURS = ("layer", "FORK.md")  # everything the layer adds to upstream's tree
AMAKE = "base/tools/bin/windows_x64/amake.exe"
BINARIES = ("base/tools/bin", "*.exe", "*.dll", "*.lib")  # programs and libraries in upstream's tree
DEFAULT_INTO = Path.home() / ".singerie" / "tools" / "armorpaint-layer-watch"


class Failed(Exception):
    """A step that stops the watch; its message goes in the report."""


def git(*args: str, cwd: Path = REPO, check: bool = True) -> str:
    result = subprocess.run(["git", *args], cwd=cwd, capture_output=True, text=True, check=False)
    if check and result.returncode != 0:
        raise Failed(f"git {' '.join(args)}: {result.stderr.strip() or result.stdout.strip()}")
    return result.stdout.strip()


def run_step(command: list[str], cwd: Path, log: Path) -> tuple[bool, str]:
    """Run one step, its output in `log`; returns whether it passed and its last lines."""
    started = time.monotonic()
    with open(log, "w", encoding="utf-8", errors="replace") as out:
        status = subprocess.run(command, cwd=cwd, stdout=out, stderr=subprocess.STDOUT, check=False).returncode
    text = log.read_text(encoding="utf-8", errors="replace")
    tail = "\n".join(text.splitlines()[-40:])
    print(f"   exit {status} in {time.monotonic() - started:.0f} s (log: {log})", flush=True)
    return status == 0, tail


def pinned() -> dict[str, str]:
    with open(PIN, "rb") as file:
        return tomllib.load(file)["upstream"]


def manifest_version(tree: Path) -> str:
    found = re.search(r'manifest_version\s*=\s*"([^"]+)"', (tree / "paint" / "sources" / "globals.h").read_text(encoding="utf-8"))
    return found.group(1) if found else "unknown"


def set_pin(commit: str, day: str, version: str, amake_sha256: str | None = None) -> None:
    text = PIN.read_text(encoding="utf-8")
    text = re.sub(r'(?m)^commit = ".*"$', f'commit = "{commit}"', text)
    text = re.sub(r'(?m)^date = ".*"$', f'date = "{day}"', text)
    text = re.sub(r'(?m)^version = ".*"$', f'version = "{version}"', text)
    if amake_sha256:
        text = re.sub(r'(?m)^amake_sha256 = ".*"$', f'amake_sha256 = "{amake_sha256}"', text)
    PIN.write_text(text, encoding="utf-8", newline="\n")


def changed_binaries(pin: str, target: str) -> list[str]:
    """The programs and libraries upstream added, changed or removed between the pin and `target`."""
    return [line for line in git("diff", "--name-status", pin, target, "--", *BINARIES).splitlines() if line.strip()]


def message(subject: str, body: str, trailers: list[str]) -> str:
    return subject + ("\n\n" + body if body else "") + ("\n\n" + "\n".join(trailers) if trailers else "")


def watch(args: argparse.Namespace) -> int:
    into = Path(args.into).resolve()
    into.mkdir(parents=True, exist_ok=True)
    report: list[str] = [f"# Upstream watch, {date.today().isoformat()}", ""]
    if args.apply:
        if git("rev-parse", "--abbrev-ref", "HEAD") != BRANCH:
            raise Failed(f"--apply needs {BRANCH} checked out.")
        if git("status", "--porcelain"):
            raise Failed("--apply needs a clean working tree (commit or set aside your changes first).")

    print("1. Fetching upstream...", flush=True)
    git("fetch", "--filter=blob:none", "upstream", "main")
    target = git("rev-parse", "upstream/main")
    pin = pinned()["commit"]
    if target == pin:
        print(f"Up to date: the pin is upstream's newest main ({pin[:7]}).")
        return 0
    for older in (pin, "main"):
        if subprocess.run(["git", "merge-base", "--is-ancestor", older, target], cwd=REPO).returncode != 0:
            raise Failed(f"Upstream's main ({target[:7]}) doesn't contain {older}: rewritten history? Look before following.")
    commits = git("log", "--format=%h %ad %s", "--date=short", f"{pin}..{target}").splitlines()
    day = git("log", "-1", "--format=%cd", "--date=short", target)
    report += [f"Pin: `{pin[:7]}`. Upstream's main: `{target[:7]}` ({day}), {len(commits)} commits newer:", ""]
    report += [f"    {line}" for line in commits[:60]] + ([f"    ... and {len(commits) - 60} more"] if len(commits) > 60 else [])
    report.append("")
    binaries = changed_binaries(pin, target)
    report += ["## Programs and libraries upstream changed", ""]
    report += [f"    {line}" for line in binaries] if binaries else ["none"]
    report += [""] + (["Read what changed before following: the build runs upstream's build tool. "
                       + ("Accepted with --accept-tools." if args.accept_tools else "Not run without --accept-tools."), ""]
                      if binaries else [])
    if binaries:
        print(f"   upstream changed {len(binaries)} program(s) or librar(ies) since the pin: listed in the report", flush=True)
    print(f"   upstream's main is {target[:7]} ({day}), {len(commits)} commits after the pin {pin[:7]}", flush=True)

    tree = Path(tempfile.mkdtemp(prefix="sng-watch-"))
    shutil.rmtree(tree)
    results: list[tuple[str, bool]] = []
    snapshot_new = into / "api_snapshot.h"
    try:
        print("2. A worktree of upstream's commit with the layer in it...", flush=True)
        git("worktree", "add", "--detach", os.fspath(tree), target)
        git("checkout", BRANCH, "--", *OURS, cwd=tree)
        version = manifest_version(tree)

        print("3. Building...", flush=True)
        build = [sys.executable, os.fspath(tree / "layer/tools/build_windows.py"), "--into", os.fspath(into)]
        ok, tail = run_step(build + (["--accept-tools"] if args.accept_tools else []), tree, into / "watch-build.log")
        results.append(("build", ok))
        report += ["## Build", "", "passed" if ok else "**failed**:", ""] + ([] if ok else ["```", tail, "```", ""])
        app = into / "armorpaint"

        if ok:
            print("4. The smoke test...", flush=True)
            snapshot_new.unlink(missing_ok=True)
            command = [sys.executable, os.fspath(tree / "layer/tests/smoke/smoke.py"), "--armorpaint", os.fspath(app)]
            ok, tail = run_step(command + ["--allow-additions", "--snapshot-out", os.fspath(snapshot_new)], tree, into / "watch-smoke.log")
            results.append(("smoke test", ok))
            log = (into / "watch-smoke.log").read_text(encoding="utf-8", errors="replace")
            api = [line.strip() for line in log.splitlines() if re.match(r"\s+(added|removed|changed): ", line)]
            report += ["## Smoke test", "", "passed" if ok else "**failed**:", ""]
            report += ([] if ok else ["```", tail, "```", ""]) + [line for line in log.splitlines() if line.startswith("ok   upstream's")]
            report += ["", f"API entries new or different ({len(api)}):", ""] + [f"    {line}" for line in api[:80]] + [""]

            if args.singerie:
                print("5. Singerie's pipeline plugin test...", flush=True)
                singerie = Path(args.singerie).resolve()
                python = args.singerie_python or next(
                    (os.fspath(p) for p in (singerie / ".venv/Scripts/python.exe",) if p.is_file()), sys.executable
                )
                test = singerie / "plugins/armorpaint/tests/run_tests.py"
                ok, tail = run_step([python, os.fspath(test), "--armorpaint", os.fspath(app)], singerie, into / "watch-pipeline.log")
                results.append(("pipeline plugin test", ok))
                report += ["## Singerie's pipeline plugin test", "", "passed" if ok else "**failed**:", ""] + ([] if ok else ["```", tail, "```", ""])
    finally:
        git("worktree", "remove", "--force", os.fspath(tree), check=False)
        git("worktree", "prune", check=False)

    passed = all(ok for _, ok in results) and len(results) >= 2
    if binaries and not args.accept_tools:
        passed = False  # never followed without a person having read what changed in upstream's programs
    report += ["## Result", "", "Everything passed." if passed else "Failed: " + ", ".join(n for n, ok in results if not ok), ""]
    (into / "watch-report.md").write_text("\n".join(report), encoding="utf-8")
    print(f"\n{'Everything passed.' if passed else 'A step failed.'} Report: {into / 'watch-report.md'}", flush=True)
    if not passed:
        return 1
    if not args.apply:
        print("Nothing changed in the repository (add --apply to follow upstream).")
        return 0

    print("6. Following upstream...", flush=True)
    git("branch", "-f", "main", target)  # a fast-forward: checked above that main is an ancestor of target
    body = f"{len(commits)} upstream commits, {day}. Built and smoke-tested by layer/tools/watch.py" + (
        ", with Singerie's pipeline plugin test." if args.singerie else "."
    )
    git("merge", "--no-ff", "-m", message(f"Follow upstream {target[:7]}: merge main into {BRANCH}", body, args.trailer), "main")
    tool = hashlib.sha256(subprocess.run(["git", "show", f"{target}:{AMAKE}"], cwd=REPO, capture_output=True, check=True).stdout).hexdigest()
    set_pin(target, day, version, tool)
    if snapshot_new.exists():
        shutil.copy2(snapshot_new, SNAPSHOT)
    git("add", os.fspath(PIN), os.fspath(SNAPSHOT))
    git("commit", "-m", message(f"Layer: pin upstream {target[:7]} ({day})", "The API snapshot follows (new entries only).", args.trailer))
    print(f"   main -> {target[:7]}, {BRANCH} merged, pin and snapshot committed.", flush=True)
    if args.push:
        git("push", "origin", "main", BRANCH)
        print("   pushed main and sng/main to origin.", flush=True)
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--into", default=os.fspath(DEFAULT_INTO), help="where to build (default %(default)s)")
    parser.add_argument("--singerie", help="Singerie's repository: also run its ArmorPaint pipeline plugin test")
    parser.add_argument("--singerie-python", help="the Python for that test (default: the repository's .venv)")
    parser.add_argument("--apply", action="store_true", help="follow upstream when everything passed")
    parser.add_argument("--push", action="store_true", help="with --apply: push main and sng/main to origin")
    parser.add_argument("--accept-tools", action="store_true", help="build and follow even though upstream changed its programs")
    parser.add_argument("--trailer", action="append", default=[], help='a line for the commits made ("Key: value")')
    args = parser.parse_args()
    if args.push and not args.apply:
        print("--push goes with --apply.")
        return 2
    try:
        return watch(args)
    except Failed as exc:
        print(f"Can't run: {exc}")
        return 2


if __name__ == "__main__":
    sys.exit(main())
