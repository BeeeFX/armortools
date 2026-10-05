// Run by smoke.py as `ArmorPaint --background --script <this, filled in>`: says what the layer did at start.
// @REPORT@ is replaced with the report file's path.

void main() {
	char *report    = string("version=%s\napi=%d\nkeymap=%s\ntheme=%s\ndefaults=%s\nupstream=%s\nlayer=%s\nname=%s\ntitle=%s\n", sng_version(), sng_api_level(),
	                         sng_info("keymap"), sng_info("theme"), sng_info("defaults"), sng_info("upstream"), sng_info("layer"), sng_info("name"), sys_title());
	buffer_t *bytes = sys_string_to_buffer(report);
	iron_file_save_bytes("@REPORT@", bytes, 0);
}
