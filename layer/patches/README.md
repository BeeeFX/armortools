# Core patches

A core patch is a change to one of upstream ArmorPaint's own files. Having none is the goal: everything the layer
adds lives in `layer/` (data files and the native module in `layer/native/`, compiled in through the
`paint/plugins/external/` hook).

It carries **none** since 13 November 2026. From 31 October to 13 November it carried one,
`0001-restore-external-hook.patch` (15 lines in `paint/plugins/project.c` and `paint/plugins/plugins.c`), which put
back that hook after upstream removed it on 22 October 2026 (`4f08044`, "paint: cleanup"). Upstream's author restored
the hook himself, exactly as it was, in `32c7af5` ("plugins: restore external hook", 13 November 2026), in answer to
the collective's issue below: thank you. The layer followed upstream to `22097df` and dropped the patch and its
`series` line the same day.

## Offering the hook back (done)

The issue, posted by Baptiste on 12 November 2026: https://github.com/armory3d/armorpaint/issues/2185. The author's
answer, on 13 November: restored at `32c7af5`; pull requests that extend the scripting are welcome, and a clean-up of
the script API, with breaking changes, will come eventually (so the layer's smoke test keeps watching the API).

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

Then GAPS.md's two smallest pull requests, prepared on 13 November 2026 as branches of this repository, each one
commit on upstream's `22097df`, for Baptiste to open from his account (they need a GitHub fork of
armory3d/armorpaint, since this repository isn't one):

- `pr/declare-layer-functions`, "paint: declare layer functions in functions.h": `base_init_undo_layers`,
  `tab_layers_delete_layer`, `tab_layers_can_delete` and `tab_layers_can_merge_down` in `functions.h` (upstream's
  `minic_impl.c` declared three of them locally; those lines go);
- `pr/script-get-env`, "paint: add script_get_env": an environment variable for scripts, `""` when unset, next to
  upstream's `script_get_platform`.

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
