// The Singerie layer's native module (route X): stable sng_* script functions for ArmorPaint's minic plugins, the
// layer's defaults (Painter-like shortcuts and theme on a fresh config), and the name the build shows in its title
// bar ("Singepaint (based on ArmorPaint)").
//
// Upstream compiles paint/plugins/external/ in when it exists and calls external_init() from plugins_init(), after
// the config is loaded and before the UI starts; layer/tools/build_windows.py lays this folder out there. This is the
// only file of the layer that calls into ArmorPaint's internals: when upstream renames or changes something, the fix
// goes here, and the build fails loudly until it is made. Plugins call the sng_* functions, never the internals.
//
// Conventions for every sng_* function (layer/README.md > The sng_* functions):
// - layers, masks, groups and filters are all "slots" of ArmorPaint's one stack, named by their id (slot_layer_t's
//   id, unique in a project; -1 means none) and placed by their index (0 is the bottom of the stack);
// - a function that changes the project pushes ArmorPaint's own history step, exactly as the Layers tab does, so
//   Ctrl+Z undoes it;
// - an int result is 1 for done and 0 for refused (an unknown id, a slot of the wrong kind), unless it returns an id;
// - text comes back as ArmorPaint's own strings or as copies: never free them.
//
// Portable C only (clang and MSVC alike): no C23 empty initializers, no variable-length arrays.
//
// zlib licence (layer/LICENSE.md), so any function here can be offered upstream unchanged.

#include "../../sources/functions.h"
#include "iron_array.h"
#include "iron_string.h"
#include "iron_system.h"
#include "minic.h"
#include "sng_build.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define SNG_API_LEVEL      1 // raised when an existing sng_* function changes meaning; new functions don't raise it
#define SNG_LAYER_VERSION  "0.1.0"
#define SNG_DEFAULT_KEYMAP "painter.json"                     // data/keymap_presets/
#define SNG_DEFAULT_THEME  "painter.json"                     // data/themes/
#define SNG_SHOWN_NAME     "Singepaint (based on ArmorPaint)" // in the window's title bar (Baptiste, 5 October 2026)

// BEGIN internals not in upstream's headers
// ArmorPaint defines these without declaring them in a header. build_windows.py checks that each line below still
// matches upstream's definition before building, since a C prototype that no longer matches would still link.
extern bool                    config_loaded;
extern char                   *manifest_title;
extern f32                     layers_default_rough;
extern i32                     base_bits;
extern char                   *ui_files_filename;
extern string_array_t         *box_export_files;
extern i32                     box_export_preset_index;
extern export_preset_t        *box_export_preset;
extern history_step_t_array_t *history_steps;
extern i32                     history_undos;
extern i32                     history_redos;
extern slot_layer_t_array_t   *history_undo_layers;
void                           base_init_undo_layers();
void                           tab_layers_delete_layer(slot_layer_t *l);
bool                           tab_layers_can_delete(slot_layer_t *l);
bool                           tab_layers_can_merge_down(slot_layer_t *l);
// END internals not in upstream's headers

static bool sng_defaults_applied = false;

// ████ Helpers ████

static slot_layer_t_array_t *sng_layers(void) {
	project_t *project = script_get_project();
	return project != NULL && project->_ != NULL ? project->_->layers : NULL;
}

static slot_layer_t *sng_slot(int id) {
	slot_layer_t_array_t *layers = sng_layers();
	if (layers == NULL || id < 0) {
		return NULL;
	}
	for (int i = 0; i < layers->length; ++i) {
		if (layers->buffer[i]->id == id) {
			return layers->buffer[i];
		}
	}
	return NULL;
}

static int sng_index_of(slot_layer_t *l) {
	slot_layer_t_array_t *layers = sng_layers();
	return layers != NULL && l != NULL ? array_index_of((any_array_t *)layers, l) : -1;
}

static int sng_id(slot_layer_t *l) {
	return l != NULL ? l->id : -1;
}

static int sng_current_id(void) {
	context_t *context = script_get_context();
	return context != NULL ? sng_id(context->layer) : -1;
}

static void sng_remap_layer_nodes(slot_layer_t_array_t *pointers) {
	// Layer and Layer Mask nodes point at slots by index: the Layers tab remaps them after a slot is inserted.
	project_t *project = script_get_project();
	for (int i = 0; i < project->_->materials->length; ++i) {
		slot_material_t *m = project->_->materials->buffer[i];
		tab_layers_remap_layer_pointers(m->canvas->nodes, tab_layers_fill_layer_map(pointers));
	}
}

// Functions that render into layers end ArmorPaint's current drawing first and resume it after, as upstream's own
// script functions do (script_gpu_begin in minic_impl.c): a script can run while a frame is being drawn.
typedef struct {
	gpu_texture_t *current;
	bool           in_use;
} sng_gpu_t;

static sng_gpu_t sng_gpu_begin(void) {
	sng_gpu_t state;
	state.current = _draw_current;
	state.in_use  = gpu_in_use;
	if (state.in_use) {
		draw_end();
	}
	return state;
}

static void sng_gpu_end(sng_gpu_t state) {
	if (state.in_use) {
		draw_begin(state.current, false, 0);
	}
}

// ArmorPaint makes its undo layers on its second frame (base_init_undo_layers in base.c); history steps that copy a
// layer's pixels need them, so a script running earlier (--script runs on the first frame) gets them made first.
static void sng_need_undo(void) {
	if (history_undo_layers == NULL) {
		base_init_undo_layers();
	}
}

static char *sng_copy(const char *text) {
	return string_copy((char *)(text != NULL ? text : ""));
}

static minic_val_t sng_text(const char *text) {
	return minic_val_typed_ptr((void *)(text != NULL ? text : ""), MINIC_T_CHAR);
}

static bool sng_is(const char *a, const char *b) {
	return a != NULL && b != NULL && strcmp(a, b) == 0;
}

static const char *sng_blend_names[] = {"mix",        "darken",       "multiply",   "burn",     "lighten", "screen", "dodge",      "add",   "overlay",
                                        "soft_light", "linear_light", "difference", "subtract", "divide",  "hue",    "saturation", "color", "value"};
#define SNG_BLEND_COUNT ((int)(sizeof(sng_blend_names) / sizeof(sng_blend_names[0])))

static const char *sng_export_mode_names[] = {"visible", "selected", "per_object", "per_udim_tile"};
#define SNG_EXPORT_MODE_COUNT ((int)(sizeof(sng_export_mode_names) / sizeof(sng_export_mode_names[0])))

// ████ About the layer ████

// sng_version(): "sng 0.1.0 (api 1, layer <commit>, ArmorPaint <commit>, built <date>)".
static minic_val_t sng_version(minic_val_t *a, int n) {
	static char *version = NULL;
	if (version == NULL) {
		version = string("sng %s (api %d, layer %s, ArmorPaint %s, built %s)", SNG_LAYER_VERSION, SNG_API_LEVEL, SNG_LAYER_COMMIT, SNG_UPSTREAM_COMMIT,
		                 SNG_BUILD_DATE);
	}
	return sng_text(version);
}

static minic_val_t sng_api_level(minic_val_t *a, int n) {
	return minic_val_int(SNG_API_LEVEL);
}

// sng_info(what): "name" (the name the build shows), "upstream", "layer", "built", "keymap", "theme", "defaults"
// ("applied" when this start gave a fresh config the layer's keymap and theme, else "kept").
static minic_val_t sng_info(minic_val_t *a, int n) {
	const char *what   = minic_arg_p(a, n, 0);
	config_t   *config = script_get_config();
	if (sng_is(what, "name")) {
		return sng_text(SNG_SHOWN_NAME);
	}
	if (sng_is(what, "upstream")) {
		return sng_text(SNG_UPSTREAM_COMMIT);
	}
	if (sng_is(what, "layer")) {
		return sng_text(SNG_LAYER_COMMIT);
	}
	if (sng_is(what, "built")) {
		return sng_text(SNG_BUILD_DATE);
	}
	if (sng_is(what, "keymap")) {
		return sng_text(config->keymap);
	}
	if (sng_is(what, "theme")) {
		return sng_text(config->theme);
	}
	if (sng_is(what, "defaults")) {
		return sng_text(sng_defaults_applied ? "applied" : "kept");
	}
	return sng_text("");
}

// sng_getenv(name): the environment variable's value, "" when it isn't set (minic has no getenv).
static minic_val_t sng_getenv(minic_val_t *a, int n) {
	const char *name  = minic_arg_p(a, n, 0);
	const char *value = name != NULL ? getenv(name) : NULL;
	return sng_text(sng_copy(value));
}

// sng_redraw(): redraw ArmorPaint's whole interface (a plugin's panel redraws only when something happens in it).
static minic_val_t sng_redraw(minic_val_t *a, int n) {
	base_redraw_ui();
	return minic_val_int(1);
}

// ████ Slots: reading ████

static minic_val_t sng_layer_count(minic_val_t *a, int n) {
	slot_layer_t_array_t *layers = sng_layers();
	return minic_val_int(layers != NULL ? layers->length : 0);
}

// sng_layer_at(index): the id of the slot at that index (0: the bottom of the stack), -1 when there is none.
static minic_val_t sng_layer_at(minic_val_t *a, int n) {
	slot_layer_t_array_t *layers = sng_layers();
	int                   index  = minic_arg_i(a, n, 0);
	if (layers == NULL || index < 0 || index >= layers->length) {
		return minic_val_int(-1);
	}
	return minic_val_int(layers->buffer[index]->id);
}

static minic_val_t sng_layer_index(minic_val_t *a, int n) {
	return minic_val_int(sng_index_of(sng_slot(minic_arg_i(a, n, 0))));
}

static minic_val_t sng_layer_current(minic_val_t *a, int n) {
	return minic_val_int(sng_current_id());
}

static minic_val_t sng_layer_select(minic_val_t *a, int n) {
	slot_layer_t *l = sng_slot(minic_arg_i(a, n, 0));
	if (l == NULL) {
		return minic_val_int(0);
	}
	context_set_layer(l);
	return minic_val_int(1);
}

static minic_val_t sng_layer_name(minic_val_t *a, int n) {
	slot_layer_t *l = sng_slot(minic_arg_i(a, n, 0));
	return sng_text(l != NULL ? l->name : "");
}

// sng_layer_kind(id): "paint", "fill", "decal", "path", "text", "group", "mask", "fill_mask", "group_mask",
// "filter", or "" for an unknown id.
static minic_val_t sng_layer_kind(minic_val_t *a, int n) {
	slot_layer_t *l = sng_slot(minic_arg_i(a, n, 0));
	if (l == NULL) {
		return sng_text("");
	}
	if (slot_layer_is_group(l)) {
		return sng_text("group");
	}
	if (slot_layer_is_mask(l)) {
		if (slot_layer_is_group_mask(l)) {
			return sng_text("group_mask");
		}
		return sng_text(l->fill_material != NULL ? "fill_mask" : "mask");
	}
	if (slot_layer_is_filter(l)) {
		return sng_text("filter");
	}
	if (l->path_text) {
		return sng_text("text");
	}
	if (slot_layer_is_path(l)) {
		return sng_text("path");
	}
	if (l->fill_material != NULL) {
		return sng_text(l->uv_type == UV_TYPE_PROJECT ? "decal" : "fill");
	}
	return sng_text("paint");
}

static minic_val_t sng_layer_parent(minic_val_t *a, int n) {
	slot_layer_t *l = sng_slot(minic_arg_i(a, n, 0));
	return minic_val_int(l != NULL ? sng_id(l->parent) : -1);
}

static minic_val_t sng_layer_opacity(minic_val_t *a, int n) {
	slot_layer_t *l = sng_slot(minic_arg_i(a, n, 0));
	return minic_val_float(l != NULL ? l->mask_opacity : 0.0f);
}

static minic_val_t sng_layer_blending(minic_val_t *a, int n) {
	slot_layer_t *l = sng_slot(minic_arg_i(a, n, 0));
	if (l == NULL || (int)l->blending < 0 || (int)l->blending >= SNG_BLEND_COUNT) {
		return sng_text("");
	}
	return sng_text(sng_blend_names[l->blending]);
}

static minic_val_t sng_layer_visible(minic_val_t *a, int n) {
	slot_layer_t *l = sng_slot(minic_arg_i(a, n, 0));
	return minic_val_int(l != NULL && l->visible ? 1 : 0);
}

// ████ Slots: creating ████

// sng_layer_new_paint(): a paint layer above the current one (as the Layers tab's New > Paint Layer, cleared at once
// rather than on the next frame, so a script can paint, merge or delete it right away); its id.
static minic_val_t sng_layer_new_paint(minic_val_t *a, int n) {
	sng_need_undo();
	slot_layer_t *l = layers_new_layer(false, -1, NULL);
	if (l == NULL) {
		return minic_val_int(-1); // ArmorPaint's layer limit
	}
	sng_gpu_t gpu = sng_gpu_begin();
	slot_layer_clear(l, 0x00000000, NULL, 1.0, layers_default_rough, 0.0);
	sng_gpu_end(gpu);
	history_new_layer();
	return minic_val_int(l->id);
}

// sng_layer_new_fill(): a fill layer above the current one, filled with the current material (New > Fill Layer,
// made at once instead of on the next frame); its id.
static minic_val_t sng_layer_new_fill(minic_val_t *a, int n) {
	sng_need_undo();
	context_t    *context = script_get_context();
	slot_layer_t *l       = layers_new_layer(false, -1, NULL);
	if (l == NULL) {
		return minic_val_int(-1);
	}
	history_new_layer();
	l->uv_type     = UV_TYPE_UVMAP;
	l->object_mask = context->layer_filter;
	sng_gpu_t gpu  = sng_gpu_begin();
	history_to_fill_layer();
	slot_layer_to_fill_layer(l);
	sng_gpu_end(gpu);
	return minic_val_int(l->id);
}

// sng_layer_group(id): put a top-level layer in a new group (New > Group with that layer selected); the group's id.
static minic_val_t sng_layer_group(minic_val_t *a, int n) {
	sng_need_undo();
	slot_layer_t *l = sng_slot(minic_arg_i(a, n, 0));
	if (l == NULL) {
		return minic_val_int(-1);
	}
	if (slot_layer_is_layer_mask(l)) {
		l = l->parent;
	}
	if (slot_layer_is_group(l) || slot_layer_is_in_group(l) || slot_layer_is_mask(l) || slot_layer_is_filter(l)) {
		return minic_val_int(-1); // groups don't nest in ArmorPaint
	}
	slot_layer_t_array_t *layers   = sng_layers();
	slot_layer_t_array_t *pointers = tab_layers_init_layer_map();
	slot_layer_t         *group    = layers_new_group();
	if (group == NULL) {
		return minic_val_int(-1);
	}
	context_set_layer(l);
	array_remove((any_array_t *)layers, group);
	array_insert((any_array_t *)layers, array_index_of((any_array_t *)layers, l) + 1, group);
	l->parent = group;
	sng_remap_layer_nodes(pointers);
	context_set_layer(group);
	history_new_group();
	return minic_val_int(group->id);
}

// sng_mask_new(id, kind): a mask on a layer or a group (on a mask's or a filter's layer when given one); kind is
// "black", "white" or "fill" (filled with the current material). The mask's id. Made whole at once (the Layers tab
// clears it on the next frame), so nothing is left waiting on a slot a script may delete in the same frame.
static minic_val_t sng_mask_new(minic_val_t *a, int n) {
	sng_need_undo();
	slot_layer_t *l    = sng_slot(minic_arg_i(a, n, 0));
	const char   *kind = minic_arg_p(a, n, 1);
	if (l == NULL || !(sng_is(kind, "black") || sng_is(kind, "white") || sng_is(kind, "fill"))) {
		return minic_val_int(-1);
	}
	if (slot_layer_is_mask(l) || slot_layer_is_filter(l)) {
		l = l->parent;
	}
	context_set_layer(l);
	slot_layer_t_array_t *pointers = tab_layers_init_layer_map();
	slot_layer_t         *m        = layers_new_mask(false, l, -1);
	if (m == NULL) {
		return minic_val_int(-1);
	}
	sng_remap_layer_nodes(pointers);
	script_get_context()->layer_preview_dirty = true;
	sng_gpu_t gpu                             = sng_gpu_begin();
	if (sng_is(kind, "black")) {
		slot_layer_clear(m, 0x00000000, NULL, 1.0, layers_default_rough, 0.0);
		history_new_black_mask();
	}
	else if (sng_is(kind, "white")) {
		slot_layer_clear(m, 0xffffffff, NULL, 1.0, layers_default_rough, 0.0);
		history_new_white_mask();
	}
	else {
		slot_layer_to_fill_layer(m);
		history_new_fill_mask();
	}
	layers_update_fill_layers();
	sng_gpu_end(gpu);
	return minic_val_int(m->id);
}

// sng_layer_duplicate(id): a copy above it, with its masks (a group: with its layers); the copy's id.
static minic_val_t sng_layer_duplicate(minic_val_t *a, int n) {
	sng_need_undo();
	slot_layer_t *l = sng_slot(minic_arg_i(a, n, 0));
	if (l == NULL || slot_layer_is_mask(l)) {
		return minic_val_int(-1);
	}
	sng_gpu_t gpu = sng_gpu_begin();
	context_set_layer(l);
	history_duplicate_layer();
	layers_duplicate_layer(l);
	sng_gpu_end(gpu);
	return minic_val_int(sng_current_id());
}

// ████ Slots: changing ████

static minic_val_t sng_layer_set_name(minic_val_t *a, int n) {
	sng_need_undo();
	slot_layer_t *l    = sng_slot(minic_arg_i(a, n, 0));
	const char   *name = minic_arg_p(a, n, 1);
	if (l == NULL || name == NULL || name[0] == '\0') {
		return minic_val_int(0);
	}
	char *previous = l->name;
	char *renamed  = sng_copy(name);
	tab_stages_rename_layer(previous, renamed);
	l->name = renamed;
	history_layer_name(l, previous);
	return minic_val_int(1);
}

static minic_val_t sng_layer_set_opacity(minic_val_t *a, int n) {
	sng_need_undo();
	slot_layer_t *l       = sng_slot(minic_arg_i(a, n, 0));
	float         opacity = minic_arg_f(a, n, 1);
	if (l == NULL) {
		return minic_val_int(0);
	}
	context_set_layer(l);
	history_layer_opacity();
	l->mask_opacity = opacity < 0.0f ? 0.0f : opacity > 1.0f ? 1.0f : opacity;
	make_material_parse_mesh_material();
	return minic_val_int(1);
}

// sng_layer_set_blending(id, mode): mode is one of the Layers tab's blending names in lower case with underscores
// ("mix", "multiply", "soft_light"...).
static minic_val_t sng_layer_set_blending(minic_val_t *a, int n) {
	sng_need_undo();
	slot_layer_t *l    = sng_slot(minic_arg_i(a, n, 0));
	const char   *mode = minic_arg_p(a, n, 1);
	if (l == NULL) {
		return minic_val_int(0);
	}
	for (int i = 0; i < SNG_BLEND_COUNT; ++i) {
		if (sng_is(mode, sng_blend_names[i])) {
			context_set_layer(l);
			history_layer_blending();
			l->blending = (blend_type_t)i;
			make_material_parse_mesh_material();
			return minic_val_int(1);
		}
	}
	return minic_val_int(0);
}

static minic_val_t sng_layer_set_visible(minic_val_t *a, int n) {
	sng_need_undo();
	slot_layer_t *l       = sng_slot(minic_arg_i(a, n, 0));
	bool          visible = minic_arg_i(a, n, 1) != 0;
	if (l == NULL) {
		return minic_val_int(0);
	}
	if (l->visible != visible) {
		history_layer_visible(l);
		l->visible = visible;
		make_material_parse_mesh_material();
	}
	return minic_val_int(1);
}

// sng_layer_move(id, index): move a slot to another index, with its masks and filters (as dragging it in the Layers
// tab does); 0 when ArmorPaint doesn't allow that place.
static minic_val_t sng_layer_move(minic_val_t *a, int n) {
	sng_need_undo();
	slot_layer_t *l  = sng_slot(minic_arg_i(a, n, 0));
	int           to = minic_arg_i(a, n, 1);
	if (l == NULL || !slot_layer_can_move(l, to)) {
		return minic_val_int(0);
	}
	slot_layer_move(l, to); // pushes its own history step
	make_material_parse_mesh_material();
	return minic_val_int(1);
}

static minic_val_t sng_layer_delete(minic_val_t *a, int n) {
	sng_need_undo();
	slot_layer_t *l = sng_slot(minic_arg_i(a, n, 0));
	if (l == NULL || !tab_layers_can_delete(l)) {
		return minic_val_int(0); // ArmorPaint always keeps one layer
	}
	sng_gpu_t gpu = sng_gpu_begin();
	tab_layers_delete_layer(l);
	sng_gpu_end(gpu);
	return minic_val_int(1);
}

// sng_layer_merge_down(id): merge a layer into the one below (masks into the mask below); the merged slot's id.
static minic_val_t sng_layer_merge_down(minic_val_t *a, int n) {
	sng_need_undo();
	slot_layer_t *l = sng_slot(minic_arg_i(a, n, 0));
	if (l == NULL || !tab_layers_can_merge_down(l)) {
		return minic_val_int(-1);
	}
	sng_gpu_t gpu = sng_gpu_begin();
	context_set_layer(l);
	history_merge_layers();
	layers_merge_down();
	context_t *context = script_get_context();
	if (context->layer->fill_material != NULL) {
		slot_layer_to_paint_layer(context->layer);
	}
	sng_gpu_end(gpu);
	return minic_val_int(sng_current_id());
}

static minic_val_t sng_mask_invert(minic_val_t *a, int n) {
	sng_need_undo();
	slot_layer_t *m = sng_slot(minic_arg_i(a, n, 0));
	if (m == NULL || !slot_layer_is_mask(m) || m->fill_material != NULL) {
		return minic_val_int(0);
	}
	sng_gpu_t gpu = sng_gpu_begin();
	context_set_layer(m);
	history_invert_mask();
	slot_layer_invert_mask(m);
	sng_gpu_end(gpu);
	return minic_val_int(1);
}

// sng_mask_apply(id): bake a mask into its layer (or its group's layers) and remove it.
static minic_val_t sng_mask_apply(minic_val_t *a, int n) {
	sng_need_undo();
	slot_layer_t *m = sng_slot(minic_arg_i(a, n, 0));
	if (m == NULL || !slot_layer_is_mask(m)) {
		return minic_val_int(0);
	}
	context_t *context = script_get_context();
	sng_gpu_t  gpu     = sng_gpu_begin();
	context->layer     = m;
	history_apply_mask();
	slot_layer_apply_mask(m);
	context_set_layer(m->parent);
	make_material_parse_mesh_material();
	context->layers_preview_dirty = true;
	sng_gpu_end(gpu);
	return minic_val_int(1);
}

// ████ History ████

// sng_history_count(): how many steps Ctrl+Z can undo now.
static minic_val_t sng_history_count(minic_val_t *a, int n) {
	return minic_val_int(history_undos);
}

static minic_val_t sng_history_redo_count(minic_val_t *a, int n) {
	return minic_val_int(history_redos);
}

// sng_history_step(back): the name of a step in the History tab ("New Layer", "Paint"...): 0 is the last one done,
// 1 the one before; a negative number reads the steps that can be redone (-1 the next).
static minic_val_t sng_history_step(minic_val_t *a, int n) {
	int back = minic_arg_i(a, n, 0);
	if (history_steps == NULL) {
		return sng_text("");
	}
	int index = history_steps->length - 1 - history_redos - back;
	if (index < 0 || index >= history_steps->length) {
		return sng_text("");
	}
	return sng_text(history_steps->buffer[index]->name);
}

static minic_val_t sng_history_undo(minic_val_t *a, int n) {
	sng_need_undo();
	if (history_undos <= 0) {
		return minic_val_int(0);
	}
	sng_gpu_t gpu = sng_gpu_begin();
	history_undo();
	sng_gpu_end(gpu);
	return minic_val_int(1);
}

static minic_val_t sng_history_redo(minic_val_t *a, int n) {
	sng_need_undo();
	if (history_redos <= 0) {
		return minic_val_int(0);
	}
	sng_gpu_t gpu = sng_gpu_begin();
	history_redo();
	sng_gpu_end(gpu);
	return minic_val_int(1);
}

// ████ Bake settings ████
// The settings every Bake Texture node shares (its type stays per node): "samples", "ao_strength", "ao_radius",
// "ao_offset", "curv_strength", "curv_radius", "curv_offset", "curv_smooth", "high_poly" (the mesh to bake from,
// by index), "axis" (0 xyz, 1 x, 2 y, 3 z, 4 -x, 5 -y, 6 -z) and "up_axis" (0 z, 1 y, 2 x).

static bool sng_bake_field(const char *setting, float **f, i32 **i, int **e) {
	context_t *c = script_get_context();
	*f           = NULL;
	*i           = NULL;
	*e           = NULL;
	if (sng_is(setting, "samples")) {
		*i = &c->bake_samples;
	}
	else if (sng_is(setting, "ao_strength")) {
		*f = &c->bake_ao_strength;
	}
	else if (sng_is(setting, "ao_radius")) {
		*f = &c->bake_ao_radius;
	}
	else if (sng_is(setting, "ao_offset")) {
		*f = &c->bake_ao_offset;
	}
	else if (sng_is(setting, "curv_strength")) {
		*f = &c->bake_curv_strength;
	}
	else if (sng_is(setting, "curv_radius")) {
		*f = &c->bake_curv_radius;
	}
	else if (sng_is(setting, "curv_offset")) {
		*f = &c->bake_curv_offset;
	}
	else if (sng_is(setting, "curv_smooth")) {
		*i = &c->bake_curv_smooth;
	}
	else if (sng_is(setting, "high_poly")) {
		*i = &c->bake_high_poly;
	}
	else if (sng_is(setting, "axis")) {
		*e = (int *)&c->bake_axis;
	}
	else if (sng_is(setting, "up_axis")) {
		*e = (int *)&c->bake_up_axis;
	}
	return *f != NULL || *i != NULL || *e != NULL;
}

static minic_val_t sng_bake_get(minic_val_t *a, int n) {
	float *f;
	i32   *i;
	int   *e;
	if (!sng_bake_field(minic_arg_p(a, n, 0), &f, &i, &e)) {
		return minic_val_float(-1.0f);
	}
	return minic_val_float(f != NULL ? *f : i != NULL ? (float)*i : (float)*e);
}

static minic_val_t sng_bake_set(minic_val_t *a, int n) {
	float *f;
	i32   *i;
	int   *e;
	float  value = minic_arg_f(a, n, 1);
	if (!sng_bake_field(minic_arg_p(a, n, 0), &f, &i, &e)) {
		return minic_val_int(0);
	}
	if (f != NULL) {
		*f = value;
	}
	else if (i != NULL) {
		*i = (i32)value;
	}
	else {
		int max = e == (int *)&script_get_context()->bake_axis ? BAKE_AXIS_MZ : BAKE_UP_AXIS_X;
		if ((int)value < 0 || (int)value > max) {
			return minic_val_int(0);
		}
		*e = (int)value;
	}
	return minic_val_int(1);
}

// ████ Export ████

static void sng_fetch_presets(void) {
	if (box_export_files == NULL) {
		box_export_fetch_presets();
		int i                   = string_array_index_of(box_export_files, "generic");
		box_export_preset_index = i > 0 ? i : 0;
	}
}

static minic_val_t sng_export_preset_count(minic_val_t *a, int n) {
	sng_fetch_presets();
	return minic_val_int(box_export_files->length);
}

static minic_val_t sng_export_preset_at(minic_val_t *a, int n) {
	int i = minic_arg_i(a, n, 0);
	sng_fetch_presets();
	return sng_text(i >= 0 && i < box_export_files->length ? box_export_files->buffer[i] : "");
}

// sng_export_preset(): the preset ArmorPaint exports with now (the Export window's; "generic" until one is chosen).
static minic_val_t sng_export_preset(minic_val_t *a, int n) {
	sng_fetch_presets();
	return sng_text(box_export_preset_index < box_export_files->length ? box_export_files->buffer[box_export_preset_index] : "");
}

// sng_export_set_preset(name): choose the preset, as the Export window's Preset does (data/export_presets/<name>.json).
static minic_val_t sng_export_set_preset(minic_val_t *a, int n) {
	const char *name = minic_arg_p(a, n, 0);
	sng_fetch_presets();
	int i = name != NULL ? string_array_index_of(box_export_files, (char *)name) : -1;
	if (i < 0) {
		return minic_val_int(0);
	}
	box_export_preset_index = i;
	box_export_parse_preset();
	return minic_val_int(1);
}

static minic_val_t sng_export_bits(minic_val_t *a, int n) {
	return minic_val_int(base_bits == TEXTURE_BITS_BITS8 ? 8 : base_bits == TEXTURE_BITS_BITS16 ? 16 : 32);
}

// sng_export_set_bits(bits): 8 (PNG or JPG), 16 or 32 (EXR); like the Export window's Color, this sets the
// project's bit depth and converts its layers (at once; the Export window does it on the next frame).
static minic_val_t sng_export_set_bits(minic_val_t *a, int n) {
	int bits = minic_arg_i(a, n, 0);
	i32 to   = bits == 8 ? TEXTURE_BITS_BITS8 : bits == 16 ? TEXTURE_BITS_BITS16 : bits == 32 ? TEXTURE_BITS_BITS32 : -1;
	if (to < 0) {
		return minic_val_int(0);
	}
	if (to != base_bits) {
		base_bits     = to;
		sng_gpu_t gpu = sng_gpu_begin();
		layers_set_bits();
		sng_gpu_end(gpu);
	}
	return minic_val_int(1);
}

// sng_export_set_format(format): "png" or "jpg", for 8-bit exports.
static minic_val_t sng_export_set_format(minic_val_t *a, int n) {
	const char *format  = minic_arg_p(a, n, 0);
	context_t  *context = script_get_context();
	if (sng_is(format, "png")) {
		context->format_type = TEXTURE_LDR_FORMAT_PNG;
	}
	else if (sng_is(format, "jpg")) {
		context->format_type = TEXTURE_LDR_FORMAT_JPG;
	}
	else {
		return minic_val_int(0);
	}
	return minic_val_int(1);
}

// sng_export_set_mode(mode): which layers go out, as the Export window's Layers: "visible", "selected",
// "per_object" or "per_udim_tile".
static minic_val_t sng_export_set_mode(minic_val_t *a, int n) {
	const char *mode = minic_arg_p(a, n, 0);
	for (int i = 0; i < SNG_EXPORT_MODE_COUNT; ++i) {
		if (sng_is(mode, sng_export_mode_names[i])) {
			script_get_context()->layers_export = (export_mode_t)i;
			return minic_val_int(1);
		}
	}
	return minic_val_int(0);
}

// sng_export_set_name(name): the first part of the exported files' names (<name>_<preset texture>.png), which
// ArmorPaint otherwise takes from the last project opened or saved through a dialog ("untitled" before that).
static minic_val_t sng_export_set_name(minic_val_t *a, int n) {
	const char *name = minic_arg_p(a, n, 0);
	if (name == NULL || name[0] == '\0') {
		return minic_val_int(0);
	}
	ui_files_filename = sng_copy(name);
	return minic_val_int(1);
}

static minic_val_t sng_export_name(minic_val_t *a, int n) {
	return sng_text(ui_files_filename);
}

// sng_export_textures(folder): export with the chosen preset, bit depth, format, layers and name, as the Export
// window does (ArmorPaint writes the files during this call).
static minic_val_t sng_export_textures(minic_val_t *a, int n) {
	const char *folder = minic_arg_p(a, n, 0);
	if (folder == NULL || folder[0] == '\0') {
		return minic_val_int(0);
	}
	sng_fetch_presets();
	if (box_export_preset == NULL) {
		box_export_parse_preset();
	}
	sng_gpu_t gpu = sng_gpu_begin();
	export_texture_run((char *)folder, false);
	sng_gpu_end(gpu);
	return minic_val_int(1);
}

// ████ Registration ████

typedef struct {
	const char       *name;
	const char       *sig; // minic's: return type then argument types, i int, f float, p pointer or text, v void
	minic_native_fn_t fn;
} sng_binding_t;

static const sng_binding_t sng_bindings[] = {
    {"sng_version", "p()", sng_version},
    {"sng_api_level", "i()", sng_api_level},
    {"sng_info", "p(p)", sng_info},
    {"sng_getenv", "p(p)", sng_getenv},
    {"sng_redraw", "i()", sng_redraw},
    {"sng_layer_count", "i()", sng_layer_count},
    {"sng_layer_at", "i(i)", sng_layer_at},
    {"sng_layer_index", "i(i)", sng_layer_index},
    {"sng_layer_current", "i()", sng_layer_current},
    {"sng_layer_select", "i(i)", sng_layer_select},
    {"sng_layer_name", "p(i)", sng_layer_name},
    {"sng_layer_kind", "p(i)", sng_layer_kind},
    {"sng_layer_parent", "i(i)", sng_layer_parent},
    {"sng_layer_opacity", "f(i)", sng_layer_opacity},
    {"sng_layer_blending", "p(i)", sng_layer_blending},
    {"sng_layer_visible", "i(i)", sng_layer_visible},
    {"sng_layer_new_paint", "i()", sng_layer_new_paint},
    {"sng_layer_new_fill", "i()", sng_layer_new_fill},
    {"sng_layer_group", "i(i)", sng_layer_group},
    {"sng_mask_new", "i(i,p)", sng_mask_new},
    {"sng_layer_duplicate", "i(i)", sng_layer_duplicate},
    {"sng_layer_set_name", "i(i,p)", sng_layer_set_name},
    {"sng_layer_set_opacity", "i(i,f)", sng_layer_set_opacity},
    {"sng_layer_set_blending", "i(i,p)", sng_layer_set_blending},
    {"sng_layer_set_visible", "i(i,i)", sng_layer_set_visible},
    {"sng_layer_move", "i(i,i)", sng_layer_move},
    {"sng_layer_delete", "i(i)", sng_layer_delete},
    {"sng_layer_merge_down", "i(i)", sng_layer_merge_down},
    {"sng_mask_invert", "i(i)", sng_mask_invert},
    {"sng_mask_apply", "i(i)", sng_mask_apply},
    {"sng_history_count", "i()", sng_history_count},
    {"sng_history_redo_count", "i()", sng_history_redo_count},
    {"sng_history_step", "p(i)", sng_history_step},
    {"sng_history_undo", "i()", sng_history_undo},
    {"sng_history_redo", "i()", sng_history_redo},
    {"sng_bake_get", "f(p)", sng_bake_get},
    {"sng_bake_set", "i(p,f)", sng_bake_set},
    {"sng_export_preset_count", "i()", sng_export_preset_count},
    {"sng_export_preset_at", "p(i)", sng_export_preset_at},
    {"sng_export_preset", "p()", sng_export_preset},
    {"sng_export_set_preset", "i(p)", sng_export_set_preset},
    {"sng_export_bits", "i()", sng_export_bits},
    {"sng_export_set_bits", "i(i)", sng_export_set_bits},
    {"sng_export_set_format", "i(p)", sng_export_set_format},
    {"sng_export_set_mode", "i(p)", sng_export_set_mode},
    {"sng_export_set_name", "i(p)", sng_export_set_name},
    {"sng_export_name", "p()", sng_export_name},
    {"sng_export_textures", "i(p)", sng_export_textures},
};

// ████ Defaults ████

// On a fresh config (first start of this build, or after ArmorPaint discarded a config written by another build),
// the layer's keymap and theme become the defaults. A config.json ArmorPaint read is never changed: whatever the
// member chose stays. Nothing happens when the preset files aren't in data/.
static void sng_apply_defaults(void) {
	if (config_loaded) {
		return;
	}
	config_t *config = script_get_config();
	if (data_get_blob("keymap_presets/" SNG_DEFAULT_KEYMAP) != NULL) {
		config->keymap = sng_copy(SNG_DEFAULT_KEYMAP);
		keymap_load();
		sng_defaults_applied = true;
	}
	if (data_get_blob("themes/" SNG_DEFAULT_THEME) != NULL) {
		config->theme        = sng_copy(SNG_DEFAULT_THEME); // base_init() loads it right after plugins_init()
		sng_defaults_applied = true;
	}
}

// ████ The shown name ████

// ArmorPaint writes its title as "<project> - ArmorPaint", "<project>* - ArmorPaint" once there are unsaved changes
// (history_push), from manifest_title. The build shows "<project> - Singepaint (based on ArmorPaint)" instead: the
// first title is set when the module starts (the window already exists, titled "untitled - ArmorPaint" by
// config_get_options, which doesn't record it where sys_title() reads), and the title is renamed on every frame after
// ArmorPaint set it again (saving, opening, the first change). manifest_title itself stays: upstream also uses it for
// the About box ("ArmorPaint.org - v..."), its bug report link and the app's data folder, which keep upstream's name.
// On Windows, nothing reads the title back but plugins (Singerie's looks for "* - ").
static void sng_title_rename(void) {
	// Runs every frame: compares in place, allocates nothing.
	char  *title  = sys_title();
	size_t length = strlen(title);
	size_t name   = strlen(manifest_title);
	if (length < name + 3 || strcmp(title + length - name, manifest_title) != 0 || strncmp(title + length - name - 3, " - ", 3) != 0) {
		return; // not ArmorPaint's own title (already renamed, or set by a plugin)
	}
	char   renamed[1024]; // sys_title_set copies into ArmorPaint's own 1024-byte title
	size_t kept = length - name - 3;
	size_t room = sizeof(renamed) - sizeof(" - " SNG_SHOWN_NAME);
	if (kept > room) {
		kept = room;
	}
	memcpy(renamed, title, kept);
	strcpy(renamed + kept, " - " SNG_SHOWN_NAME);
	sys_title_set(renamed);
}

static void sng_title_update(void *_) {
	sng_title_rename();
}

void external_init(void) {
	for (int i = 0; i < (int)(sizeof(sng_bindings) / sizeof(sng_bindings[0])); ++i) {
		minic_register(sng_bindings[i].name, sng_bindings[i].sig, sng_bindings[i].fn);
	}
	sng_apply_defaults();
	if (sys_title()[0] == '\0') {
		sys_title_set("untitled - " SNG_SHOWN_NAME); // the window's first title, as config_get_options words it
	}
	sng_title_rename();
	sys_notify_on_update(&sng_title_update, NULL);
}
