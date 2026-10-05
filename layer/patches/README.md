# Core patches

A core patch is a change to one of upstream ArmorPaint's own files. This layer carries **none**, and that is the
goal: everything it adds lives in `layer/` (data files and the native module in `layer/native/`, compiled in through
upstream's own `paint/plugins/external/` hook).

## The budget

- **No edits to upstream files by default.**
- At most **3** patches at a time, each at most **150 changed lines**, and only **while the matching upstream pull
  request is pending**. When upstream merges it (or refuses it), the patch goes.
- When the budget is full, a new patch replaces an old one or waits.

Why so strict: upstream's `main` changes every day, and every carried patch must be rebased onto it. A small
"expose this function to scripts" pull request is usually merged upstream within days, so a local patch should
never live long.

## The format, when one is needed

- One `git format-patch` file per patch, numbered (`0001-…patch`), listed in `series` in the order they apply.
- Each patch's message starts with a header:

  ```
  What: one sentence
  Size: <changed lines> lines in <files>
  Upstream: offered <link to the pull request>   (or: local, because <reason>)
  Gap: <GAPS.md item it serves>
  ```

- The build applies them to its copy of the tree (never to the repository itself) before compiling. The build tool
  doesn't apply patches yet: add that the day the first one is needed.

## What doesn't count as a patch

The three syntax changes `layer/tools/build_windows.py` makes when it builds with MSVC instead of clang (C23 empty
initializers, file-scope compound literals, two variable-length arrays) are made in the build's copy only, for
scratch builds used in tests. They are never committed and never reach members, whose builds are made with clang.
