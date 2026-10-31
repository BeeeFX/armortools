# What ArmorPaint misses compared with Substance 3D Painter

Ranked by how much texture artists use each thing (an estimate), with the route each takes and where it stands.
The full analysis, with sources, is in Singerie's `docs/dev/research/parity-layers.md` › ArmorPaint (27 September
2026); this list is the working copy and changes as the layer does.

Routes: **E** extension points only (data files, minic plugins, node graphs); **X** the layer's native module
(`layer/native/`, compiled in through the `paint/plugins/external/` hook; upstream removed that hook on 22 October
2026, and the layer's one core patch puts it back); **C** a core patch (an upstream file edited: avoided, see
`patches/README.md`). Painter facts come from Adobe's public
documentation only.

## Done in this layer

| What | Route | Where |
|---|---|---|
| Painter's shortcuts as the default keymap (tools 1 to 6, F to frame, Ctrl+RMB drag for size, Shift+RMB for the environment, S for the stencil, Tab to hide the interface, Ctrl+Y redo) | E + X | `ext/keymap_presets/painter.json`, made the default by `native/sng_bindings.c` |
| A Painter-like dark theme as the default | E + X | `ext/themes/painter.json` |
| Studio export presets with Singerie's map names (`basecolor_srgb`, `roughness_raw`...), upstream's Unreal and Unity presets kept | E | `ext/export_presets/` |
| Script functions for layers, masks, groups, reordering, history, bake settings and export (the adapter every later feature builds on) | X | `native/sng_bindings.c`, the `sng_*` functions |
| A script can choose the export preset, bit depth, format, layers and file name (plugins couldn't) | X | `sng_export_*` |
| `getenv` for scripts (Singerie's pipeline plugin could drop its `.singerie` hand-over) | X | `sng_getenv` |

## Next, in order

| # | Painter feature | ArmorPaint today | Route | Size |
|---|---|---|---|---|
| 1 | **Generators** (curvature, AO, edge wear, dirt, position gradient) | Building blocks only: Bake Texture, Curvature, Gradient, Noise nodes | E: a pack of node-group presets | M |
| 2 | **Smart masks and single-graph smart materials** | A fill mask's graph can be saved as `.arm` | E: a library built from CC0 textures | S–M |
| 3 | **Multi-layer smart materials** | No: materials are single node graphs | E: a loader plugin on `sng_layer_*` and `sng_mask_new` | M |
| 4 | **Bake all maps in one click** | One map per Bake Texture node; no cage, ray distance or match by name | E: a bake panel plugin on `sng_bake_*` (running a bake from a script is the next binding) | M |
| 5 | **OCIO view in the viewport** | No OCIO; a `.cube` LUT only | E: the project's view baked to a `.cube` and loaded as the viewport LUT | S |
| 6 | **Layer shortcuts** (Delete, Ctrl+D duplicate, Ctrl+G group) | Menu items only; plugins can't add shortcuts | X: an update hook in the native module, careful with text fields | S |
| 7 | **Tabs, menus and shortcuts for plugins** (a shelf, a properties panel, the bake panel outside the Plugins tab) | Plugins draw only inside the Plugins tab | C, offered upstream as a small API | S |
| 8 | **Brush spacing and a pressure curve** | Pressure maps to size, hardness, opacity, angle; no curve | C, offered upstream | S–M |
| 9 | **Export naming tokens and packing masks into outputs** (#2134) | No tokens; the pipeline renames after export | C, offered upstream (open issue) | S–M |
| 10 | **Group masks referenced from a Layer Mask node** (#2132) | "To be implemented" upstream | C, help upstream | S |
| 11 | **Ctrl+RMB drag both ways** (horizontal size, vertical hardness) | Brush drags change one value, horizontally | C, offered upstream | S |
| 12 | **Blend mode and opacity per channel** | One mode and opacity per layer | C, a design issue upstream; never carried locally (it changes the project format) | L |

**Not planned:** a stack and resolution per texture set, custom channels, real OCIO, `.spp` import (each large, and
unlikely upstream).

## Small upstream pull requests this layer suggests

Each is a few lines and would let the layer drop a workaround. None is opened yet.

| Pull request | Why |
|---|---|
| Put back the `paint/plugins/external/` hook (`project.c` compiles the folder in when it exists, `plugins_init()` calls `external_init()`), removed in `4f08044` "paint: cleanup" on 22 October 2026 | The layer's native module comes in through it; until then the layer carries it as its one core patch (`patches/0001-restore-external-hook.patch`, 15 lines) |
| Declare `tab_layers_delete_layer`, `tab_layers_can_delete`, `tab_layers_can_merge_down` and `base_init_undo_layers` in `functions.h` | The bindings declare them by hand (checked by the build, but a header is safer) |
| Script functions for the layer stack: create a paint or fill layer, a mask, a group; delete, move, duplicate, merge; opacity, blending, visibility, name | The core of `sng_layer_*`; upstream merges such binding PRs within days |
| Script functions for history (undo, redo, the steps' names) | `sng_history_*` |
| Script functions to choose the export preset and file name | `sng_export_set_preset`, `sng_export_set_name` |
| `getenv` for scripts | `sng_getenv` |
| Don't toggle distraction-free mode while typing in a text field (`util_shortcut_global`) | With Tab as that shortcut (Painter's), typing Tab in a field would hide the interface |
| Make the undo layers before the first script runs (or guard history steps that copy pixels) | A script run with `--script` reached history before ArmorPaint's second frame and crashed on missing undo layers |
| Undo of a move over several places swaps two slots instead of moving back | Seen in `history_undo` for `HISTORY_ACTION_ORDER_LAYERS` (to confirm with a repro first) |
| Keep the preferences across builds | ArmorPaint discards `config.json` whenever the build's commit changes, so every update resets a member's preferences |
