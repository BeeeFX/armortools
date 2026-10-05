# The Singerie layer for ArmorPaint

This folder makes ArmorPaint feel like Substance 3D Painter, for the artists of the collective that uses Singerie,
**without changing any of upstream ArmorPaint's own files**. Everything here is added beside them: data files
ArmorPaint reads (a keymap, a theme, export presets) and a small native module upstream compiles in through its
own "external" hook. So the layer follows upstream: when ArmorPaint moves on, the layer is rebuilt on the new
commit, and a smoke test says at once what broke. `../FORK.md` says the same in two paragraphs; `GAPS.md` lists what
Painter has that ArmorPaint doesn't, and what comes next.

Singerie's pipeline plugin for ArmorPaint (the Singerie panel, the relay, publishing) is not here: it lives in
Singerie's own repository (`plugins/armorpaint`). This repository holds only changes and new features of the app.

## What's where

| Path | What |
|---|---|
| `upstream.toml` | The upstream commit the layer is tested against, and the toolchain |
| `ext/keymap_presets/painter.json` | Painter's shortcuts, as an ArmorPaint keymap preset |
| `ext/themes/painter.json` | A Painter-like dark theme |
| `ext/export_presets/singerie*.json` | The studio's texture export presets |
| `native/sng_bindings.c` | The native module: the `sng_*` script functions and the layer's defaults. The only file that calls ArmorPaint's internals |
| `native/sng_build.h` | What is being built (the build tool rewrites it in its copy) |
| `patches/` | Core patches: none (`series` is empty; `patches/README.md` has the budget) |
| `tools/build_windows.py` | Builds ArmorPaint with the layer on Windows |
| `tests/smoke/` | The smoke test, its minic scripts and the script API snapshot |
| `GAPS.md` | Painter features ArmorPaint misses, ranked, with their route; upstream pull request ideas |
| `LICENSE.md` | zlib, the licence of the layer's own files |

## Building on Windows

```bash
python layer/tools/build_windows.py            # into ~/.singerie/tools/armorpaint-layer-build/armorpaint
python layer/tools/build_windows.py --into D:/b # elsewhere
```

It needs Visual Studio 2022 with "Desktop development with C++" and Python 3.10+; nothing is downloaded. It never
writes inside the repository: it copies `base/` and `paint/` to `<into>/src`, lays `layer/native/` out as
`paint/plugins/external/` there (upstream compiles that folder in when it exists, defines `WITH_EXTERNAL` and calls
its `external_init()` from `plugins_init()`; upstream's `.gitignore` lists the folder), runs upstream's `amake`,
builds Release x64 with MSBuild, then lays the app out in `<into>/armorpaint/` with the layer's data files merged
into `data/`, the layer's licence in `data/licenses/singerie_layer.md` and `ALTERED_VERSION.txt` saying plainly that
it is an altered ArmorPaint (zlib licence, point 2). The About box shows `<upstream>+sng.<layer>` as the commit.
Before building, it checks that the few internals the bindings declare by hand still match upstream's definitions
(a C declaration that no longer matches would still link), and after, that the repository is unchanged.

**With clang** (Visual Studio's "C++ Clang tools for Windows"), it builds exactly as upstream does: this is how
members' builds are made. Checked on 5 October 2026 with Visual Studio 2022's clang-cl 16.0.5 (ClangCL toolset) on
upstream `4c06b93`: it builds without any change (about 25 s for MSBuild), and the smoke test and Singerie's
pipeline plugin checks pass on it. `--toolset clang` insists on it; `auto` (the default) takes it when installed.

**Without clang**, it switches the copy to MSVC (toolset v143) and makes three
mechanical syntax changes MSVC needs, in the copy only: C23 empty initializers `(void *[]){}` become
`(void *[]){0}` (each is passed with a length of 0), compound literals initializing file-scope variables lose their
cast, and the two variable-length arrays of the colour ramp and vector curves nodes get a fixed size of 1024. They
change nothing ArmorPaint does, are never committed, and such a build is for tests only (`ALTERED_VERSION.txt` says
so). The layer's own code is portable C and needs none of them; the build refuses it if it ever does.

Logs go to `<into>/logs/` (`amake.log`, `msbuild.log`). A build takes about 15 seconds.

## Packaging a members' build

```bash
python layer/tools/build_windows.py --toolset clang
python layer/tools/package_windows.py --out <folder>
```

`package_windows.py` packs a clang build of the repository's current commit as the archive Singerie's software
manager installs, and refuses anything else (an MSVC scratch build, a build of another commit, uncommitted changes).
It writes `singepaint-<version>-win64.zip` (one folder holding the app; a `config.json` or session folder a run left
in the build stays out, so a member's first start gets the layer's defaults), `<zip>.sha256`, and `<zip>.json` with
what an admin enters in Singerie's Admin › Desktop app › Builds of our apps (app `armorpaint`, version, archive name,
size, SHA-256, plus the commits and toolset). It then unpacks the archive with Windows' own `tar.exe`, as the
software manager does, and starts the app once (`--api` must list the `sng_*` functions).

The version is shaped the way the software manager reads and orders builds (three numbers, then `+` and a build part
of letters, digits, dots and dashes; ordered by all their numbers): `<ArmorPaint's version>+<upstream
date>.sng<n>.<upstream>.<layer>`, for example `1.1.0+2026.10.04.sng16.4c06b93.64a2092`. ArmorPaint's version comes
from its `manifest_version` (`1.1alpha` is `1.1.0`: a project pins `1.1.x` for it); the upstream commit's date orders
builds of different upstream commits; `n`, the count of the layer's own commits, orders builds of one upstream commit;
the two commits at the end are the About box's `<upstream>+sng.<layer>`. The script uploads nothing: publishing is an
admin's step in Singerie.

## Testing

```bash
python layer/tests/smoke/smoke.py              # the build above (or --armorpaint <folder holding ArmorPaint.exe>)
python layer/tests/smoke/smoke.py --static     # only the checks that need no build
```

Without a build, against this tree's upstream sources: the theme has ArmorPaint's fields in their order (its theme
reader is positional, not by name); the keymap names only actions ArmorPaint has (an upstream rename would silently
drop a shortcut) and gives no plain key to two actions (ArmorPaint would fire both); the export presets have
upstream's shape, only channels ArmorPaint exports, and names that say their colour space; no layer file replaces
an upstream one.

With the build, each part in a fresh copy of its folder:

- the layer's data files are in the app;
- **the name**: the title bar says `untitled - Singepaint (based on ArmorPaint)` from the start, and
  `untitled* - Singepaint (based on ArmorPaint)` once a change marks the project unsaved;
- **the defaults**: a first start writes the Painter keymap and theme in ArmorPaint's `data/config.json`; a config
  with the member's own choice keeps it;
- **`--api`**: all `sng_*` functions registered, and ArmorPaint's own script API (its functions, structs, enums,
  globals and material nodes, 865 entries at the pinned commit) compared with `tests/smoke/api_snapshot.h`: an
  upstream rename, removal or new parameter is listed by name;
- **the bindings** (`tests/smoke/scripts/bindings.c`, run with `--background --script`): every `sng_*` function on
  a fresh project (a merge, a paint layer renamed, faded, blended, hidden then shown through undo and redo, a fill
  layer moved and the move undone and redone, black and white masks, a group, a duplicate deleted, a mask inverted
  and another applied, bake settings, refusals), then an export with the studio preset: nine 2048-pixel PNGs named
  `smoke_basecolor_srgb.png`, `smoke_normal_raw.png`...

Singerie's pipeline plugin has its own real-app test (`plugins/armorpaint/tests/run_tests.py --armorpaint
<folder>` in Singerie's repository); it runs against this build like against a plain one.

## The Painter keymap

`ext/keymap_presets/painter.json` is a partial ArmorPaint keymap: what it doesn't name keeps ArmorPaint's default.
Painter's shortcuts are taken from Adobe's public documentation (Substance 3D Painter › Interface › Settings ›
Shortcuts); nothing else of Painter is used.

| Action | ArmorPaint's default | Painter preset | Painter's own |
|---|---|---|---|
| Camera rotate, pan, zoom | Alt+LMB, Alt+MMB, Alt+RMB | (same) | the same |
| Rotate the environment | Ctrl+MMB | **Shift+RMB** | Shift+RMB |
| Brush size (drag) | F | **Ctrl+RMB** (horizontal) | Ctrl+RMB (horizontal size, vertical hardness) |
| Brush opacity (drag) | Shift+F | **Ctrl+LMB** (horizontal) | Ctrl+LMB (horizontal flow, vertical rotation) |
| Brush angle (drag) | Alt+F | **Ctrl+MMB** | (vertical Ctrl+LMB) |
| Brush size step | [ and ] | (same) | [ and ] |
| Straight line | Shift | (same) | Shift+LMB |
| Stencil: move, scale, rotate | Ctrl | **S** | S + mouse |
| Stencil: hide | Z | **N** | N (ignore the stencil) |
| Decal mask | Ctrl | **Shift** | (none; Ctrl now drags opacity) |
| Paint, eraser, projection (decal), polygon fill, smudge (blur), clone | B, E, D, G, U, L | **1, 2, 3, 4, 5, 6** | 1 to 6 |
| Colour picker | V | **P** | P (pick the stroke's material) |
| Particle, Colour ID, Select tools | P, C, M | **none** (the toolbar) | Painter's Ctrl+1 can't be told from 1 by ArmorPaint |
| Frame the mesh | 0 | **F** | F (centre the mesh) |
| Front, back, side, top views; orbit steps | numbers and Ctrl+numbers | **none** (the View menu) | the number keys are tools |
| Perspective / orthographic | 5 | **F6** | F5 / F6 (F5 is ArmorPaint's player, hard-wired) |
| Hide the interface | F11 | **Tab** | Tab |
| 2D view, node editor | Shift+Tab, Tab | **F1, F2** | F1: the 3D/2D layout |
| Viewport display mode menu | Ctrl+M | **C** (then the mode's letter) | C cycles the channels shown |
| Redo | Ctrl+Shift+Z | **Ctrl+Y** | Ctrl+Y |
| Undo, save, open, new, export textures (Ctrl+Shift+E) | | (same) | the same |

Known limits, all in ArmorPaint's input code (so not fixable from the layer without a core patch): a brush drag
changes one value, horizontally only; a shortcut without modifiers also fires with them (which is why views can't
move to Ctrl+numbers while tools are on the numbers); the distraction-free toggle isn't guarded against typing, so
Tab typed in a text field hides the interface; F5 always starts ArmorPaint's player. Painter's Delete, Ctrl+D and
Ctrl+G for layers have no ArmorPaint action (GAPS.md, item 6). These bindings were reasoned from ArmorPaint's input
code and checked to load; they haven't been tried by hand in the window yet.

## The name: Singepaint

The collective's build shows **"Singepaint (based on ArmorPaint)"** in its title bar (Baptiste, 5 October 2026):
`chr_monkey_texturing_main_v001* - Singepaint (based on ArmorPaint)`. ArmorPaint writes its title from its
`manifest_title` ("ArmorPaint") when it starts, saves, opens and on the first change; the native module renames
the title's ` - ArmorPaint` ending on the frame after (and sets the window's first title itself, since ArmorPaint
doesn't record that one). `manifest_title` itself stays, because upstream also uses it for the About box
("ArmorPaint.org - v..."), its bug report link and the folder of its settings: those keep upstream's name, and the
About box's commit says `+sng.`. No upstream file is changed. `sng_info("name")` gives the name to scripts. On
Windows nothing reads the title back except plugins: Singerie's looks for `* - ` (unsaved changes), which works with
either name. The program is still `ArmorPaint.exe`, which Singerie's software manager and launcher expect.

## The theme

`ext/themes/painter.json`: neutral dark greys with a blue accent, flat buttons, no shadows; ArmorPaint's sizes. Its
fields must be in ArmorPaint's order (`ui_theme_keys` in `base/sources/iron_ui_ext.c`), all whole numbers; colours
are ARGB written as ArmorPaint writes them (signed). In hex: window `ff252526`, hover `ff3c3c3e`, button
`ff323234`, pressed `ff1b1b1c`, text `ffe8e8e8`, labels `ffaaaaaa`, separators `ff161617`, highlight `ff2f80d1`,
viewport `ff3a3a3a`. It is our own palette, inspired by the look of Painter's dark interface, not copied from it.

## The defaults

On a fresh config (`config.json` absent, or one ArmorPaint discarded), the native module makes the Painter keymap
and theme the defaults before ArmorPaint's interface starts; ArmorPaint then saves them in `data/config.json` as
usual. A config ArmorPaint read is never changed: a member who chooses another keymap or theme in Preferences keeps
it. Two ArmorPaint behaviours to know: it **discards its config whenever the build's commit changes** (so after
each update of the collective's build, every preference, ours included, starts again from the defaults: a candidate
upstream change, GAPS.md), and editing shortcuts in Preferences writes them into the selected preset's own file
(`data/keymap_presets/painter.json` in the install).

## The studio export presets

| Preset | Maps |
|---|---|
| `singerie` | `basecolor_srgb`, `normal_raw` (OpenGL), `occlusion_raw`, `roughness_raw`, `metallic_raw`, `height_raw`, `opacity_raw`, `emission_mask_raw`, `subsurface_raw`: every channel Singerie's texture sets take, one map each, as the collective's Cycles and Karma materials want |
| `singerie_packed` | `basecolor_srgb` (opacity in alpha), `normal_raw` (OpenGL), `orm_raw` (occlusion, roughness, metallic) |

Names follow Singerie's colour rule: the word after the channel is the colour space the map is in, `srgb` for colour
maps and `raw` for data. ArmorPaint writes `<name>_<map>.png` (or `.exr` for 16 and 32 bits), so an export named
`chr_monkey_texture` gives `chr_monkey_texture_basecolor_srgb.png`. Every map is `"color_space": "linear"`, which in
ArmorPaint means "written as painted": its colours are the sRGB values picked in its colour wheel, which is what
`_srgb` says; any other value there would convert the pixels again. ArmorPaint's emission and subsurface are masks
(how much, not a colour), hence `emission_mask_raw`. Upstream's presets (Unreal, Unity and the others) are kept;
the Export window still starts on `generic`. A plugin chooses a preset with `sng_export_set_preset`.

## The sng_* functions

Registered for every minic script and plugin by `native/sng_bindings.c` (48 at API level 1). Conventions: slots
(layers, masks, groups, filters: ArmorPaint keeps them in one stack) are named by **id** (`-1` for none) and placed
by **index** (0 at the bottom); every change pushes ArmorPaint's own history step, as the Layers tab does, so Ctrl+Z
undoes it; an `int` result is 1 for done and 0 for refused unless it returns an id; returned text is never freed.
Creating a layer or a mask finishes within the call (the Layers tab clears it on the next frame), so a script can
use or delete it at once.

| Function | What |
|---|---|
| `char *sng_version()`, `int sng_api_level()` | `sng 0.1.0 (api 1, layer …, ArmorPaint …, built …)`; the level rises only when a function changes meaning |
| `char *sng_info(char *what)` | `name` ("Singepaint (based on ArmorPaint)"), `upstream`, `layer`, `built`, `keymap`, `theme`, `defaults` (`applied` or `kept` this start) |
| `char *sng_getenv(char *name)` | An environment variable, `""` when unset (minic has no getenv) |
| `int sng_redraw()` | Redraw the whole interface |
| `int sng_layer_count()`, `int sng_layer_at(int index)`, `int sng_layer_index(int id)` | The stack |
| `int sng_layer_current()`, `int sng_layer_select(int id)` | The selected slot |
| `char *sng_layer_name(int id)`, `char *sng_layer_kind(int id)`, `int sng_layer_parent(int id)` | Kind: `paint`, `fill`, `decal`, `path`, `text`, `group`, `mask`, `fill_mask`, `group_mask`, `filter` |
| `float sng_layer_opacity(int id)`, `char *sng_layer_blending(int id)`, `int sng_layer_visible(int id)` | Blending names: `mix`, `darken`, `multiply`, `burn`, `lighten`, `screen`, `dodge`, `add`, `overlay`, `soft_light`, `linear_light`, `difference`, `subtract`, `divide`, `hue`, `saturation`, `color`, `value` |
| `int sng_layer_new_paint()`, `int sng_layer_new_fill()` | A layer above the selected one (a fill layer with the current material); its id |
| `int sng_layer_group(int id)` | Put a top-level layer in a new group; the group's id |
| `int sng_mask_new(int id, char *kind)` | `black`, `white` or `fill` mask on a layer or group; its id |
| `int sng_layer_duplicate(int id)` | A copy with its masks; its id |
| `int sng_layer_set_name(int id, char *name)`, `int sng_layer_set_opacity(int id, float opacity)`, `int sng_layer_set_blending(int id, char *mode)`, `int sng_layer_set_visible(int id, int visible)` | |
| `int sng_layer_move(int id, int index)` | With its masks and filters, where ArmorPaint allows it |
| `int sng_layer_delete(int id)` | Refused for the last layer |
| `int sng_layer_merge_down(int id)` | The merged slot's id |
| `int sng_mask_invert(int id)`, `int sng_mask_apply(int id)` | |
| `int sng_history_count()`, `int sng_history_redo_count()`, `char *sng_history_step(int back)` | Steps Ctrl+Z can undo and redo; a step's name (0: the last done, -1: the next redo) |
| `int sng_history_undo()`, `int sng_history_redo()` | |
| `float sng_bake_get(char *setting)`, `int sng_bake_set(char *setting, float value)` | The settings every Bake Texture node shares: `samples`, `ao_strength`, `ao_radius`, `ao_offset`, `curv_strength`, `curv_radius`, `curv_offset`, `curv_smooth`, `high_poly`, `axis`, `up_axis` |
| `int sng_export_preset_count()`, `char *sng_export_preset_at(int i)`, `char *sng_export_preset()`, `int sng_export_set_preset(char *name)` | The Export window's preset |
| `int sng_export_bits()`, `int sng_export_set_bits(int bits)` | 8, 16 or 32 (the project's bit depth, as the Export window's Color) |
| `int sng_export_set_format(char *format)`, `int sng_export_set_mode(char *mode)` | `png` or `jpg`; `visible`, `selected`, `per_object`, `per_udim_tile` |
| `int sng_export_set_name(char *name)`, `char *sng_export_name()` | The files' first part (ArmorPaint otherwise uses the last project opened through a dialog, or "untitled") |
| `int sng_export_textures(char *folder)` | Export now, with all of the above |

Plugins should call `sng_*` functions plus a short list of upstream's own, so that an upstream change is fixed once,
in the bindings. minic's limits still apply to plugins: 32 functions, 64 globals, an 8 MB arena, no preprocessor;
`string()` returns memory of the calling function, so a global keeps a `string_copy`.

## Keeping up with upstream

The layer follows upstream ArmorPaint's `main` **weekly, once the smoke test passes** (Baptiste, 5 October 2026).
GitHub Actions are off on this repository, so the watch runs from a PC that can build (Visual Studio 2022):

```bash
python layer/tools/watch.py --singerie <Singerie's repository>                  # check: nothing changes
python layer/tools/watch.py --singerie <Singerie's repository> --apply --push   # follow upstream, push
```

It fetches upstream; if `main` moved past the pin, it builds the layer on upstream's newest commit in a temporary
worktree, runs the smoke test (new upstream API entries pass; removed or changed ones fail and are named) and, with
`--singerie`, Singerie's pipeline plugin test (`plugins/armorpaint/tests/run_tests.py`) on that build, then writes
`watch-report.md` beside the build (`~/.singerie/tools/armorpaint-layer-watch`): the upstream commits since the
pin, each step's result, the API's new entries. With `--apply`, and only when every step passed, it fast-forwards
`main` to upstream, merges `main` into `sng/main` (no conflict is possible: the layer touches no upstream file),
sets the pin in `upstream.toml`, writes the new `api_snapshot.h`, and commits; `--push` pushes both branches
(never forced). It needs `sng/main` checked out with nothing uncommitted. `--trailer "Key: value"` adds a line to
its commits.

When a step fails, nothing changes: read the report and the logs beside it. A compile error or a failed internals
check points at `native/sng_bindings.c`; a failed static check at a data file in `ext/`; a removed or changed API
entry at whatever uses it (the bindings, Singerie's pipeline plugin). Fix it on `sng/main`, run the watch again.

To run it every week, Windows' Task Scheduler can start it, for example on Mondays at 9:00 (one line, in a terminal
on the PC that builds; it only reports, without `--apply`):

```bat
schtasks /Create /SC WEEKLY /D MON /ST 09:00 /TN "Singepaint upstream watch" /TR "python %USERPROFILE%\.singerie\tools\armortools-layer\layer\tools\watch.py --singerie <Singerie's repository>"
```

The first follow (5 October 2026): `main` fast-forwarded from `8c8e336` to `4c06b93` (58 upstream commits, several
of them script API changes); the layer built unchanged, the smoke test passed with 70 new upstream functions
(procedural geometry, environment maps, a few maths functions) and nothing removed or changed, and Singerie's
pipeline plugin test passed (56 checks).

## Licence

zlib (`LICENSE.md`), like ArmorPaint, so any piece can be offered upstream unchanged. Builds are marked as altered
versions "based on ArmorPaint" (`ALTERED_VERSION.txt`, the `+sng.` commit in the About box) and keep ArmorPaint's
licence and notices.
