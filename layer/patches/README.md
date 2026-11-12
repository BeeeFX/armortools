# Core patches

A core patch is a change to one of upstream ArmorPaint's own files. Having none is the goal: everything the layer
adds lives in `layer/` (data files and the native module in `layer/native/`, compiled in through the
`paint/plugins/external/` hook).

It carries **one** since 31 October 2026: `0001-restore-external-hook.patch` (15 lines in `paint/plugins/project.c`
and `paint/plugins/plugins.c`) puts back that hook, exactly as upstream had it until it removed it on 22 October
2026 (`4f08044`, "paint: cleanup"). Without it the native module, and so every `sng_*` function, the defaults and the
Singepaint title, wouldn't be compiled in. It is "local" for now. Baptiste agreed on 12 November 2026 that the
collective offers it back, starting with a short issue (below); once posted, its link goes in this paragraph and in the
patch header's `Upstream:` line. If upstream takes it back, delete the file and its line in `series`.

## Offering the hook back

The issue, posted by Baptiste on 12 November 2026: https://github.com/armory3d/armorpaint/issues/2185

> **Title:** Could the `paint/plugins/external` hook come back?
>
> Hello, and thank you for ArmorPaint. We are a small animation collective that builds ArmorPaint from source for our
> texture artists, with a few additions compiled in through the `paint/plugins/external/` hook: `project.c` compiled
> that folder in when it existed, and `plugins_init()` called its `external_init()`. Commit 4f08044 ("paint:
> cleanup", 22 October 2026) removed those 15 lines, and we now carry them back as a local patch. Would you consider
> keeping the hook? It costs nothing when the folder isn't there, and it lets a fork add native script functions
> without touching any of your files. If you'd rather not, we understand. We'd also be glad to offer some of those
> functions as small pull requests (layer stack, history and export settings for scripts), and to declare
> `tab_layers_delete_layer`, `tab_layers_can_delete`, `tab_layers_can_merge_down` and `base_init_undo_layers` in
> `functions.h`, if that's welcome.
>
> The lines, as they were:
>
> ```c
> // paint/plugins/project.c, before `return project;`
> if (fs_exists("external")) {
> 	add_cfiles(project, "external/**");
> 	add_shaders(project, "external/*.shader");
> 	add_assets(project, "external/assets/*", "data/{name}", 0);
> 	add_define(project, "WITH_EXTERNAL");
> }
>
> // paint/plugins/plugins.c, after bake_maps_init(); in plugins_init()
> #ifdef WITH_EXTERNAL
> 	external_init();
> #endif
> ```

Then, whatever the answer: GAPS.md's two smallest pull requests (the four declarations in `functions.h`; `getenv` for
scripts), and the answer recorded here.

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

- The build applies them to its copy of the tree (never to the repository itself) before compiling, with
  `git apply`, in `series` order; one that no longer applies stops the build with git's message (refresh it on the new
  upstream commit: `git worktree add --detach <tmp> <commit>`, make the change, commit, `git format-patch -1`).

## What doesn't count as a patch

The three syntax changes `layer/tools/build_windows.py` makes when it builds with MSVC instead of clang (C23 empty
initializers, file-scope compound literals, two variable-length arrays) are made in the build's copy only, for
scratch builds used in tests. They are never committed and never reach members, whose builds are made with clang.
