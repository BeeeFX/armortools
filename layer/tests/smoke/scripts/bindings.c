// Run by smoke.py as `ArmorPaint --background --script <this, filled in>` on a fresh project (the default mesh and
// one paint layer): every sng_* function once, over three frames (masks invert and apply on a later frame than the
// one they're made in, as with the Layers tab), then exports: the studio preset, the packed one into another folder,
// and the studio preset again at 16 bits (EXR). Writes "key=value" lines; smoke.py checks them and the pictures.
// @REPORT@, @EXPORT@, @EXPORT_PACKED@ and @EXPORT_16@ are replaced with the report file's path and three folders,
// @MISSING@ with a path that isn't there.
// minic: at most 32 functions besides main, main last; literals aren't kept in globals (string() copies are).

char *report;
int   first_id;
int   paint_id;
int   fill_id;
int   mask_black;
int   mask_white;
int   group_id;
int   dup_id;

// The report is written again after every line, so a crash still shows how far the script got. string() gives
// memory of the function calling it, given back when it returns: the global keeps a copy.
void put(char *key, char *value) {
	report          = string_copy(string("%s%s=%s\n", report, key, value));
	buffer_t *bytes = sys_string_to_buffer(report);
	iron_file_save_bytes("@REPORT@", bytes, 0);
}

void put_i(char *key, int value) {
	put(key, string("%d", value));
}

void put_f(char *key, float value) {
	put(key, string("%.3f", value));
}

void finish() {
	put("done", "yes");
}

void step_export() {
	put_i("export_set_preset", sng_export_set_preset("singerie"));
	put_i("export_set_preset_unknown", sng_export_set_preset("nonsense"));
	put("export_preset", sng_export_preset());
	put_i("export_set_name", sng_export_set_name("smoke"));
	put("export_name", sng_export_name());
	put_i("export_set_mode", sng_export_set_mode("visible"));
	put_i("export_set_mode_unknown", sng_export_set_mode("nonsense"));
	put_i("export_set_format", sng_export_set_format("png"));
	put_i("export_set_bits_refused", sng_export_set_bits(12));
	put_i("export_bits", sng_export_bits());
	put_i("export_textures_missing_folder", sng_export_textures("@MISSING@"));
	put_i("export_textures", sng_export_textures("@EXPORT@"));
	put_i("export_set_packed", sng_export_set_preset("singerie_packed"));
	put_i("export_textures_packed", sng_export_textures("@EXPORT_PACKED@"));
	sng_export_set_preset("singerie");
	put_i("export_set_bits_16", sng_export_set_bits(16));
	put_i("export_textures_16", sng_export_textures("@EXPORT_16@"));
	finish();
}

// The Export window read back and set: Layers, Format, Destination (what Singerie's publish puts back afterwards), also
// through sng_info, which a plugin that must load in older builds uses.
void step_export_window() {
	put("export_mode_start", sng_export_mode());
	put("export_format_start", sng_export_format());
	put("export_destination_start", sng_export_destination());
	sng_export_set_mode("selected");
	sng_export_set_format("jpg");
	put("export_mode_selected", sng_export_mode());
	put("info_export_mode", sng_info("export_mode"));
	put("info_export_format", sng_info("export_format"));
	put_i("export_set_destination", sng_export_set_destination("project"));
	put("export_destination_project", sng_export_destination());
	put("info_export_destination", sng_info("export_destination"));
	put_i("export_set_destination_unknown", sng_export_set_destination("nonsense"));
	sng_export_set_destination("disk");
	put("info_unknown", sng_info("nonsense"));
}

void step_masks() {
	// The title on the frame after the changes above: ArmorPaint marked it unsaved, the layer renamed it.
	put("title_after_change", sys_title());
	put_i("mask_invert", sng_mask_invert(mask_black));
	put("history_after_invert", sng_history_step(0));
	put_i("mask_invert_refused", sng_mask_invert(paint_id));
	put_i("mask_apply", sng_mask_apply(mask_white));
	put("kind_white_after_apply", sng_layer_kind(mask_white));
	put("history_after_apply", sng_history_step(0));
	put_i("count_after_apply", sng_layer_count());
	step_export_window();
	script_notify_on_next_frame(step_export);
}

void main() {
	report = string_copy("");
	put("version", sng_version());
	put("title_start", sys_title());

	// The fresh project: one paint layer. Merge a new layer down into it, and try to delete the last layer.
	put_i("count_start", sng_layer_count());
	first_id = sng_layer_at(0);
	put("kind_start", sng_layer_kind(first_id));
	put("name_start", sng_layer_name(first_id));
	put_i("at_out_of_range", sng_layer_at(99));
	put("kind_unknown", sng_layer_kind(9999));
	put_i("delete_last_refused", sng_layer_delete(first_id));
	put_i("select_first", sng_layer_select(first_id));
	int top = sng_layer_new_paint();
	put_i("count_before_merge", sng_layer_count());
	put_i("merged_is_first", sng_layer_merge_down(top) == first_id);
	put_i("count_after_merge", sng_layer_count());
	put("history_after_merge", sng_history_step(0));
	put_i("merge_refused", sng_layer_merge_down(first_id));

	// A paint layer: name, opacity, blending, visibility.
	paint_id = sng_layer_new_paint();
	put("kind_paint", sng_layer_kind(paint_id));
	put_i("current_is_paint", sng_layer_current() == paint_id);
	put_i("index_paint", sng_layer_index(paint_id));
	put("history_after_paint", sng_history_step(0));
	put_i("set_name", sng_layer_set_name(paint_id, "Rust"));
	put("name_paint", sng_layer_name(paint_id));
	put("history_after_name", sng_history_step(0));
	put_i("set_opacity_nan_refused", sng_layer_set_opacity(paint_id, sqrt(-1.0)));
	put_i("set_opacity", sng_layer_set_opacity(paint_id, 0.5));
	put_f("opacity_paint", sng_layer_opacity(paint_id));
	put_i("set_blending", sng_layer_set_blending(paint_id, "multiply"));
	put("blending_paint", sng_layer_blending(paint_id));
	put_i("set_blending_unknown", sng_layer_set_blending(paint_id, "nonsense"));
	put_i("set_visible_off", sng_layer_set_visible(paint_id, 0));
	put_i("visible_paint_off", sng_layer_visible(paint_id));
	put_i("undo_visible", sng_history_undo());
	put_i("visible_paint_undone", sng_layer_visible(paint_id));
	put_i("redo_count", sng_history_redo_count());
	put("history_redo_next", sng_history_step(-1));
	put_i("redo_visible", sng_history_redo());
	put_i("visible_paint_redone", sng_layer_visible(paint_id));
	sng_layer_set_visible(paint_id, 1);

	// A fill layer above it, moved one place down, the move undone and redone.
	fill_id = sng_layer_new_fill();
	put("kind_fill", sng_layer_kind(fill_id));
	put_i("index_fill", sng_layer_index(fill_id));
	put("history_after_fill", sng_history_step(0));
	int fill_index = sng_layer_index(fill_id);
	put_i("move", sng_layer_move(fill_id, fill_index - 1));
	put_i("index_fill_moved", sng_layer_index(fill_id));
	put("history_after_move", sng_history_step(0));
	put_i("undo_move", sng_history_undo());
	put_i("index_fill_undone", sng_layer_index(fill_id));
	put_i("redo_move", sng_history_redo());
	put_i("index_fill_redone", sng_layer_index(fill_id));
	put_i("move_refused", sng_layer_move(fill_id, 999));

	// Masks, a group, a duplicate deleted.
	mask_black = sng_mask_new(paint_id, "black");
	put("kind_mask_black", sng_layer_kind(mask_black));
	put_i("parent_mask_black_is_paint", sng_layer_parent(mask_black) == paint_id);
	put("history_after_mask", sng_history_step(0));
	mask_white = sng_mask_new(fill_id, "white");
	put_i("parent_mask_white_is_fill", sng_layer_parent(mask_white) == fill_id);
	put_i("mask_refused", sng_mask_new(paint_id, "grey"));
	group_id = sng_layer_group(paint_id);
	put("kind_group", sng_layer_kind(group_id));
	put_i("parent_paint_is_group", sng_layer_parent(paint_id) == group_id);
	put("history_after_group", sng_history_step(0));
	put_i("group_refused", sng_layer_group(-5));
	dup_id = sng_layer_duplicate(fill_id);
	put("kind_dup", sng_layer_kind(dup_id));
	put_i("dup_is_new", dup_id != fill_id);
	put_i("delete_dup", sng_layer_delete(dup_id));
	put("kind_dup_after_delete", sng_layer_kind(dup_id));
	put("history_after_delete", sng_history_step(0));
	put_i("history_count", sng_history_count());

	// Bake settings.
	put_i("bake_set_samples", sng_bake_set("samples", 64));
	put_f("bake_samples", sng_bake_get("samples"));
	put_i("bake_set_ao_radius", sng_bake_set("ao_radius", 0.25));
	put_f("bake_ao_radius", sng_bake_get("ao_radius"));
	put_i("bake_set_axis", sng_bake_set("axis", 3));
	put_f("bake_axis", sng_bake_get("axis"));
	put_i("bake_set_axis_refused", sng_bake_set("axis", 9));
	put_i("bake_set_unknown", sng_bake_set("nonsense", 1));
	put_i("bake_set_samples_refused", sng_bake_set("samples", 0));
	put_i("bake_set_high_poly", sng_bake_set("high_poly", 0));
	put_i("bake_set_high_poly_refused", sng_bake_set("high_poly", 5));
	put_i("bake_set_high_poly_negative", sng_bake_set("high_poly", -1));
	put_f("bake_get_unknown", sng_bake_get("nonsense"));

	// The rest.
	put_i("preset_count", sng_export_preset_count());
	put("preset_start", sng_export_preset());
	put("getenv", sng_getenv("SNG_SMOKE"));
	put("getenv_unset", sng_getenv("SNG_SMOKE_UNSET_VARIABLE"));
	put_i("asset_repoint_unknown", sng_asset_repoint("@MISSING@", "@MISSING@"));
	put_i("mesh_reimport_missing", sng_mesh_reimport("@MISSING@"));
	put_i("redraw", sng_redraw());
	put_i("count_end_of_frame", sng_layer_count());
	script_notify_on_next_frame(step_masks);
}
