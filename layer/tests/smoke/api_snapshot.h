// ArmorPaint script API

typedef struct i8_array_t {
    char * buffer;
    int length;
    int capacity;
} i8_array_t;

typedef struct u8_array_t {
    char * buffer;
    int length;
    int capacity;
} u8_array_t;

typedef struct i16_array_t {
    void * buffer;
    int length;
    int capacity;
} i16_array_t;

typedef struct u16_array_t {
    void * buffer;
    int length;
    int capacity;
} u16_array_t;

typedef struct i32_array_t {
    void * buffer;
    int length;
    int capacity;
} i32_array_t;

typedef struct u32_array_t {
    void * buffer;
    int length;
    int capacity;
} u32_array_t;

typedef struct f32_array_t {
    void * buffer;
    int length;
    int capacity;
} f32_array_t;

typedef struct any_array_t {
    void * buffer;
    int length;
    int capacity;
} any_array_t;

typedef struct string_array_t {
    void * buffer;
    int length;
    int capacity;
} string_array_t;

typedef struct buffer_t {
    char * buffer;
    int length;
    int capacity;
} buffer_t;

typedef struct vec2_t {
    float x;
    float y;
} vec2_t;

typedef struct vec3_t {
    float x;
    float y;
    float z;
} vec3_t;

typedef struct vec4_t {
    float x;
    float y;
    float z;
    float w;
} vec4_t;

typedef struct quat_t {
    float x;
    float y;
    float z;
    float w;
} quat_t;

typedef struct mat3_t {
    float m00;
    float m01;
    float m02;
    float m10;
    float m11;
    float m12;
    float m20;
    float m21;
    float m22;
} mat3_t;

typedef struct mat4_t {
    float m00;
    float m01;
    float m02;
    float m03;
    float m10;
    float m11;
    float m12;
    float m13;
    float m20;
    float m21;
    float m22;
    float m23;
    float m30;
    float m31;
    float m32;
    float m33;
} mat4_t;

typedef struct ui_window_t {
    int layout;
    float scroll_offset;
    int redraws;
    char * text;
    bool scroll_enabled;
    bool drag_enabled;
} ui_window_t;

typedef struct ui_color_state_t {
    float hue;
    float sat;
    float val;
    float red;
    float green;
    float blue;
    float alpha;
    int mode;
} ui_color_state_t;

typedef struct ui_node_socket_t {
    int id;
    int node_id;
    char * name;
    char * type;
    int color;
    f32_array_t *default_value;
    float min;
    float max;
    float precision;
    int display;
} ui_node_socket_t;

typedef struct ui_node_button_t {
    char * name;
    char * type;
    int output;
    f32_array_t *default_value;
    u8_array_t *data;
    float min;
    float max;
    float precision;
    float height;
} ui_node_button_t;

typedef struct ui_node_link_t {
    int id;
    int from_id;
    int from_socket;
    int to_id;
    int to_socket;
} ui_node_link_t;

typedef struct ui_node_t {
    int id;
    char * name;
    char * type;
    float x;
    float y;
    int color;
    any_array_t *inputs;
    any_array_t *outputs;
    any_array_t *buttons;
    float width;
    int flags;
} ui_node_t;

typedef struct ui_node_canvas_t {
    char * name;
    any_array_t *nodes;
    any_array_t *links;
} ui_node_canvas_t;

typedef struct slot_layer_t {
    int id;
    char * name;
    bool visible;
    slot_layer_t *parent;
    float mask_opacity;
    slot_material_t *fill_material;
    int object_mask;
    float scale;
    float angle;
    int blending;
    f32_array_t *path_points;
    bool path_curved;
    bool path_text;
    slot_material_t *path_material;
    gpu_texture_t *texpaint_sculpt;
    bool paint_base;
    bool paint_opac;
    bool paint_occ;
    bool paint_rough;
    bool paint_met;
    bool paint_nor;
    bool paint_nor_blend;
    bool paint_height;
    bool paint_height_blend;
    bool paint_emis;
    bool paint_subs;
} slot_layer_t;

typedef struct slot_material_t {
    ui_node_canvas_t *canvas;
    int id;
    bool paint_base;
    bool paint_opac;
    bool paint_occ;
    bool paint_rough;
    bool paint_met;
    bool paint_nor;
    bool paint_height;
    bool paint_emis;
    bool paint_subs;
} slot_material_t;

typedef struct object_desc_t {
    char * name;
    char * type;
    char * data_ref;
    f32_array_t *transform;
    f32_array_t *dimensions;
    int visible;
    int spawn;
    void * anim;
    char * material_ref;
    object_desc_t_array_t *children;
} object_desc_t;

typedef struct vertex_array_t {
    char * attrib;
    char * data;
    i16_array_t *values;
} vertex_array_t;

typedef struct mesh_data_t {
    char * name;
    float scale_pos;
    float scale_tex;
    vertex_array_t_array_t *vertex_arrays;
    u32_array_t *index_array;
    void * _;
} mesh_data_t;

typedef struct mesh_geo_t {
    void * pos;
    void * nor;
    void * tex;
    void * ind;
    int vertex_count;
    int index_count;
} mesh_geo_t;

typedef struct mesh_geo_image_t {
    int width;
    int height;
} mesh_geo_image_t;

typedef struct camera_data_t {
    char * name;
    float near_plane;
    float far_plane;
    float fov;
    float aspect;
    bool frustum_culling;
    f32_array_t *ortho;
} camera_data_t;

typedef struct world_data_t {
    char * name;
    int color;
    float strength;
    char * irradiance;
    char * radiance;
    int radiance_mipmaps;
    char * envmap;
    void * _;
} world_data_t;

typedef struct vertex_element_t {
    char * name;
    char * data;
} vertex_element_t;

typedef struct shader_const_t {
    char * name;
    char * type;
    char * link;
} shader_const_t;

typedef struct tex_unit_t {
    char * name;
    char * link;
} tex_unit_t;

typedef struct shader_context_t {
    char * name;
    int depth_write;
    char * compare_mode;
    char * cull_mode;
    char * vertex_shader;
    char * fragment_shader;
    int shader_from_source;
    char * blend_source;
    char * blend_destination;
    char * alpha_blend_source;
    char * alpha_blend_destination;
    string_array_t *color_attachments;
    char * depth_attachment;
    vertex_element_t_array_t *vertex_elements;
    shader_const_t_array_t *constants;
    tex_unit_t_array_t *texture_units;
    bind_tex_t_array_t *bind_textures;
} shader_context_t;

typedef struct bind_tex_t {
    char * name;
    char * file;
} bind_tex_t;

typedef struct shader_data_t {
    char * name;
    any_array_t *contexts;
    void * _;
} shader_data_t;

typedef struct gpu_texture_t {
    int width;
    int height;
    int format;
} gpu_texture_t;

typedef struct render_target_t {
    char * name;
    int width;
    int height;
    char * format;
    float scale;
    void * _image;
} render_target_t;

typedef struct object_t {
    int uid;
    float urandom;
    char * name;
    transform_t *transform;
    void * parent;
    any_array_t *children;
    bool visible;
    bool is_empty;
    void * ext;
    char * ext_type;
} object_t;

typedef struct mesh_object_t {
    object_t *base;
    mesh_data_t *data;
    shader_data_t *material;
    float camera_dist;
    bool frustum_culling;
    bool culled;
} mesh_object_t;

typedef struct transform_t {
    vec4_t loc;
    quat_t rot;
    vec4_t scale;
    float scale_world;
    int dirty;
    object_t *object;
    vec4_t dim;
    vec4_t extent;
    bool has_dim;
    bool measured;
    float radius;
} transform_t;

typedef struct physics_body_t {
    int shape;
    float mass;
    float dimx;
    float dimy;
    float dimz;
    vec4_t offset;
    object_t *obj;
} physics_body_t;

typedef struct camera_object_t {
    object_t *base;
    camera_data_t *data;
    int frame;
    frustum_plane_array_t *frustum_planes;
} camera_object_t;

typedef struct config_t {
    int window_w;
    int window_h;
    float window_scale;
    float rp_supersample;
    string_array_t *recent_projects;
    string_array_t *plugins;
    char * keymap;
    char * theme;
    int undo_steps;
    float camera_fov;
    int layer_res;
    int brush_live;
    int node_previews;
    int material_live;
    int tools;
} config_t;

typedef struct context_t {
    mesh_object_t *paint_object;
    int ddirty;
    int pdirty;
    slot_material_t *material;
    slot_layer_t *layer;
    void * brush;
    int tool;
    float brush_radius;
    float brush_opacity;
    float brush_hardness;
    float brush_scale;
    float brush_angle;
    int brush_blending;
    int brush_sculpt;
    int viewport_mode;
    int xray;
    bool capturing_screenshot;
    float envmap_angle;
} context_t;

typedef struct project_t {
    char * version;
    string_array_t *assets;
    int is_bgra;
    char * envmap;
    float camera_fov;
    float camera_near;
    float camera_far;
    bool camera_frustum_culling;
    f32_array_t *camera_world;
    f32_array_t *camera_origin;
    void * swatches;
    void * brush_nodes;
    void * material_nodes;
    string_array_t *font_assets;
    void * layer_datas;
    void * mesh_datas;
    string_array_t *script_datas;
    string_array_t *script_names;
} project_t;

// Enums
#define UI_LAYOUT_VERTICAL 0
#define UI_LAYOUT_HORIZONTAL 1
#define UI_ALIGN_LEFT 0
#define UI_ALIGN_CENTER 1
#define UI_ALIGN_RIGHT 2
#define UI_STATE_IDLE 0
#define UI_STATE_STARTED 1
#define UI_STATE_DOWN 2
#define UI_STATE_RELEASED 3
#define UI_STATE_HOVERED 4
#define GPU_TEXTURE_FORMAT_RGBA32 0
#define GPU_TEXTURE_FORMAT_RGBA64 1
#define GPU_TEXTURE_FORMAT_RGBA128 2
#define GPU_TEXTURE_FORMAT_R8 3
#define GPU_TEXTURE_FORMAT_R16 4
#define GPU_TEXTURE_FORMAT_R32 5
#define GPU_TEXTURE_FORMAT_D32 6
#define GPU_TEXTURE_FORMAT_RGBA32_BC7 7
#define PHYSICS_SHAPE_BOX 0
#define PHYSICS_SHAPE_SPHERE 1
#define PHYSICS_SHAPE_TERRAIN 2
#define PHYSICS_SHAPE_MESH 3
#define RENDER_PASS_LIGHT 0
#define RENDER_PASS_POST 1
#define TOOLS_PAINT 0
#define TOOLS_SCULPT 1
#define TOOLS_OBJECT 2
#define SCULPT_TYPE_DRAW 0
#define SCULPT_TYPE_GRAB 1
#define SCULPT_TYPE_INFLATE 2
#define SCULPT_TYPE_FLATTEN 3
#define SCULPT_TYPE_CLAY 4
#define SCULPT_TYPE_PINCH 5
#define SCULPT_TYPE_CREASE 6
#define SCULPT_TYPE_CLOTH 7
#define SCULPT_TYPE_TWIST 8
#define SCULPT_TYPE_STRETCH 9
#define SCULPT_TYPE_TRIM 10
#define SCULPT_TYPE_PLATEAU 11
#define TOOL_TYPE_BRUSH 0
#define TOOL_TYPE_ERASER 1
#define TOOL_TYPE_FILL 2
#define TOOL_TYPE_DECAL 3
#define TOOL_TYPE_TEXT 4
#define TOOL_TYPE_CLONE 5
#define TOOL_TYPE_BLUR 6
#define TOOL_TYPE_PARTICLE 7
#define TOOL_TYPE_COLORID 8
#define TOOL_TYPE_PICKER 9
#define TOOL_TYPE_MATERIAL 10
#define TOOL_TYPE_CURSOR 11
#define TOOL_TYPE_SELECT 12
#define TOOL_TYPE_EMPTY 13
#define TOOL_TYPE_PLANE 14
#define TOOL_TYPE_CUBE 15
#define TOOL_TYPE_SPHERE 16
#define TOOL_TYPE_CONE 17
#define TOOL_TYPE_CYLINDER 18
#define TOOL_TYPE_CAPSULE 19
#define TOOL_TYPE_TORUS 20
#define TOOL_TYPE_TEXT_MESH 21
#define TOOL_TYPE_PATH 22
#define TOOL_TYPE_CURVE 23
#define TOOL_TYPE_BAKE 24
#define MESH_BOOLEAN_UNION 0
#define MESH_BOOLEAN_DIFFERENCE 1
#define MESH_BOOLEAN_INTERSECT 2

// Globals
extern float mouse_x;
extern float mouse_y;
extern float mouse_wheel_delta;
extern float mouse_movement_x;
extern float mouse_movement_y;

// Functions
int bake_maps_load_high(void *);
float bake_maps_high_fit(void);
int bake_maps_set_low(void);
void bake_maps_set_cage(float, bool);
void bake_maps_set_ao(float, float, float);
void bake_maps_set_curvature(float, float, float, int);
void bake_maps_set_cavity(float, float);
bool bake_maps_start(int, int, int, int, bool, int, int);
bool bake_maps_gpu_supported(void);
int bake_maps_gpu_res(void);
bool bake_maps_gpu_start(int, int, float, float, float);
float bake_maps_progress(void);
bool bake_maps_step(void);
void bake_maps_cancel(void);
bool bake_maps_save(void *, void *);
void *bake_maps_preview(int, int, float, float, float, bool, bool, bool, bool, bool);
float bake_maps_miss_ratio(void);
void bake_maps_end_images(void);
void bake_maps_end(void);
void bake_maps_ui_column(int);
void bake_maps_ui_section(void *);
void bake_maps_ui_enabled(bool);
bool bake_maps_inpaint_begin(void *);
void bake_maps_inpaint_stroke(void);
void *bake_maps_inpaint_mask(void);
void bake_maps_inpaint_clear(void);
bool bake_maps_inpaint_apply(void *, int, bool);
void bake_maps_inpaint_end(void);
int printf(char *fmt, ...);
int sprintf(char *dst, char *fmt, ...);
char *string(char *fmt, ...);
float vec2_len(vec2_t v);
vec2_t vec2_set_len(vec2_t v, float length);
vec2_t vec2_mult(vec2_t a, float f);
vec2_t vec2_add(vec2_t a, vec2_t b);
vec2_t vec2_sub(vec2_t a, vec2_t b);
float vec2_cross(vec2_t a, vec2_t b);
vec2_t vec2_norm(vec2_t v);
float vec2_dot(vec2_t a, vec2_t b);
vec2_t vec2_nan(void);
bool vec2_isnan(vec2_t v);
vec4_t vec4_cross(vec4_t a, vec4_t b);
vec4_t vec4_add(vec4_t a, vec4_t b);
vec4_t vec4_fadd(vec4_t a, float x, float y, float z, float w);
vec4_t vec4_norm(vec4_t a);
vec4_t vec4_mult(vec4_t v, float f);
float vec4_dot(vec4_t a, vec4_t b);
vec4_t vec4_apply_proj(vec4_t a, mat4_t m);
vec4_t vec4_apply_mat4(vec4_t a, mat4_t m);
vec4_t vec4_apply_axis_angle(vec4_t a, vec4_t axis, float angle);
vec4_t vec4_apply_quat(vec4_t a, quat_t q);
bool vec4_equals(vec4_t a, vec4_t b);
bool vec4_almost_equals(vec4_t a, vec4_t b, float prec);
float vec4_len(vec4_t a);
vec4_t vec4_sub(vec4_t a, vec4_t b);
float vec4_dist(vec4_t v1, vec4_t v2);
vec4_t vec4_reflect(vec4_t a, vec4_t n);
vec4_t vec4_clamp(vec4_t a, float min, float max);
vec4_t vec4_x_axis(void);
vec4_t vec4_y_axis(void);
vec4_t vec4_z_axis(void);
vec4_t vec4_nan(void);
bool vec4_isnan(vec4_t v);
quat_t quat_from_axis_angle(vec4_t axis, float angle);
quat_t quat_from_mat(mat4_t m);
quat_t quat_from_rot_mat(mat4_t m);
quat_t quat_mult(quat_t a, quat_t b);
quat_t quat_norm(quat_t q);
vec4_t quat_get_euler(quat_t q);
quat_t quat_from_euler(float x, float y, float z);
float quat_dot(quat_t a, quat_t b);
quat_t quat_from_to(vec4_t v0, vec4_t v1);
quat_t quat_inv(quat_t q);
mat3_t mat3_identity(void);
mat3_t mat3_translation(float x, float y);
mat3_t mat3_rotation(float alpha);
mat3_t mat3_scale(mat3_t m, vec4_t v);
mat3_t mat3_set_from4(mat4_t m4);
mat3_t mat3_multmat(mat3_t a, mat3_t b);
mat3_t mat3_transpose(mat3_t m);
mat3_t mat3_nan(void);
bool mat3_isnan(mat3_t m);
mat4_t mat4_identity(void);
mat4_t mat4_persp(float fov_y, float aspect, float zn, float zf);
mat4_t mat4_ortho(float left, float right, float bottom, float top, float znear, float zfar);
mat4_t mat4_rot_z(float alpha);
mat4_t mat4_compose(vec4_t loc, quat_t rot, vec4_t scl);
mat4_t mat4_set_loc(mat4_t m, vec4_t v);
mat4_t mat4_from_quat(quat_t q);
mat4_t mat4_translate(mat4_t m, float x, float y, float z);
mat4_t mat4_scale(mat4_t m, vec4_t v);
mat4_t mat4_mult_mat3x4(mat4_t a, mat4_t b);
mat4_t mat4_mult_mat(mat4_t a, mat4_t b);
mat4_t mat4_inv(mat4_t a);
mat4_t mat4_transpose(mat4_t m);
mat4_t mat4_transpose3(mat4_t m);
vec4_t mat4_get_loc(mat4_t m);
vec4_t mat4_get_scale(mat4_t m);
mat4_t mat4_mult(mat4_t m, float s);
mat4_t mat4_to_rot(mat4_t m);
vec4_t mat4_right(mat4_t m);
vec4_t mat4_look(mat4_t m);
vec4_t mat4_up(mat4_t m);
f32_array_t *mat4_to_f32_array(mat4_t m);
float mat4_determinant(mat4_t m);
mat4_t mat4_nan(void);
bool mat4_isnan(mat4_t m);
void transform_set_matrix(transform_t *raw, mat4_t mat);
void transform_rotate(transform_t *raw, vec4_t axis, float f);
void transform_move(transform_t *raw, vec4_t axis, float f);
vec4_t transform_look(transform_t *raw);
vec4_t transform_right(transform_t *raw);
vec4_t transform_up(transform_t *raw);
vec4_t raycast_aabb_mouse(object_t *object);
bool point_in_aabb(object_t *object, vec4_t point);
void script_tween_to(object_t *o, vec4_t to, float speed);
vec4_t script_physics_get_pos(object_t *o);
vec4_t script_physics_get_velocity(object_t *o);
void script_physics_set_rotation(object_t *o, quat_t rot);
vec4_t script_pick_terrain(void);
vec4_t script_physics_terrain_raycast(vec4_t origin, vec4_t dir);
vec4_t script_physics_box_raycast(vec4_t origin, vec4_t dir, float max_dist);
vec4_t camera_object_look_world(camera_object_t *raw);
void line_draw_render(mat4_t matrix);
void line_draw_bounds(mat4_t mat, vec4_t dim);
void shape_draw_sphere(mat4_t mat);
void draw_set_transform(mat3_t matrix);
int iron_random_get(void);
int iron_random_get_max(int max);
int iron_random_get_in(int min, int max);
float vec4_fdist(float v1x, float v1y, float v1z, float v2x, float v2y, float v2z);
float mat4_cofactor(float m0, float m1, float m2, float m3, float m4, float m5, float m6, float m7, float m8);
float cosf(float x);
float sinf(float x);
float sqrt(float x);
float cos(float x);
float sin(float x);
float atan2(float y, float x);
float asin(float x);
float acos(float x);
float pow(float x, float y);
float floor(float x);
float fabs(float x);
void object_set_parent(object_t *raw, object_t *parent_object);
object_t *object_get_child(object_t *raw, char *name);
void transform_reset(transform_t *raw);
void transform_update(transform_t *raw);
void transform_build_matrix(transform_t *raw);
void transform_decompose(transform_t *raw);
float transform_world_x(transform_t *raw);
float transform_world_y(transform_t *raw);
float transform_world_z(transform_t *raw);
void camera_object_build_proj(camera_object_t *raw, float screen_aspect);
void camera_object_remove(camera_object_t *raw);
void camera_object_build_mat(camera_object_t *raw);
world_data_t *world_data_parse(char *name, char *id);
void world_data_load_envmap(world_data_t *raw);
shader_data_t *shader_data_parse(char *file, char *name);
void shader_data_delete(shader_data_t *raw);
shader_context_t *shader_data_get_context(shader_data_t *raw, char *name);
void shader_context_load(shader_context_t *raw);
void shader_context_load_textures(shader_context_t *raw);
void shader_context_compile(shader_context_t *raw);
void shader_context_finish_compile(shader_context_t *raw);
void shader_context_delete(shader_context_t *raw);
void shader_context_add_const(shader_context_t *raw, int offset);
void shader_context_add_tex(shader_context_t *raw, int i);
mesh_data_t *mesh_data_parse(char *name, char *id);
int mesh_data_get_vertex_size(char *vertex_data);
void mesh_data_build_vertices(gpu_buffer_t *vertex_buffer, any_array_t *vertex_arrays);
void mesh_data_build_indices(gpu_buffer_t *index_buffer, u32_array_t *index_array);
vertex_array_t *mesh_data_get_vertex_array(mesh_data_t *raw, char *name);
void mesh_data_build(mesh_data_t *raw);
void mesh_data_delete(mesh_data_t *raw);
void mesh_object_set_data(mesh_object_t *raw, mesh_data_t *data);
void mesh_object_render(mesh_object_t *raw, shader_context_t *scontext, string_array_t *bind_params);
void mesh_object_draw(mesh_object_t *raw, shader_context_t *scontext, string_array_t *bind_params);
mesh_data_t *data_get_mesh(char *file, char *name);
camera_data_t *data_get_camera(char *file, char *name);
world_data_t *data_get_world(char *file, char *name);
shader_data_t *data_get_shader(char *file, char *name);
scene_t *data_get_scene_raw(char *file);
gpu_texture_t *data_get_texture(char *file);
buffer_t *data_get_blob(char *file);
video_t *data_get_video(char *file);
draw_font_t *data_get_font(char *file);
sound_t *data_get_sound(char *file);
void data_delete_mesh(char *handle);
void data_delete_blob(char *handle);
void data_delete_texture(char *handle);
void data_delete_video(char *handle);
void data_delete_font(char *handle);
bool data_is_abs(char *file);
char *data_path(void);
void scene_remove(void);
object_t *scene_set_active(char *scene_name);
object_t *scene_add_object(object_t *parent);
object_t *scene_get_child(char *name);
mesh_object_t *scene_add_mesh_object(mesh_data_t *data, shader_data_t *material, object_t *parent);
camera_object_t *scene_add_camera_object(camera_data_t *data, object_t *parent);
void scene_add_scene(char *scene_name);
object_t *scene_spawn_object(char *name, object_t *parent, int spawn_children);
object_desc_t *scene_get_raw_object_by_name(scene_t *format, char *name);
void scene_gen_transform(object_desc_t *object, transform_t *transform);
void render_path_set_target(char *target, string_array_t *additional, char *depth_buffer, int flags, int color, float depth);
void render_path_end(void);
void render_path_draw_skydome(char *handle);
void render_path_bind_target(char *target, char *uniform);
void render_path_draw_shader(char *handle);
void render_path_load_shader(char *handle);
void render_path_resize(void);
render_target_t *render_path_create_render_target(render_target_t *t);
render_target_t *render_target_create(void);
bool ui_item_changed(void);
ui_window_t *ui_window_create(void);
void ui_window_destroy(ui_window_t *window);
void ui_push_id(int id);
void ui_pop_id(void);
void ui_set_next_id(int id);
void ui_begin(ui_t *ui);
void ui_begin_sticky(void);
void ui_end_sticky(void);
void ui_begin_region(ui_t *ui, int x, int y, int w);
void ui_end_region(void);
bool ui_window(ui_window_t *handle, int x, int y, int w, int h, int drag);
bool ui_button(char *text, int align, char *label);
int ui_text(char *text, int align, int bg);
bool ui_icon_button(char *text, int icon, int align);
bool ui_tab(int *value, char *text, int vertical, int color, int align_right);
bool ui_tab_changed(ui_window_t *window, int *value);
bool ui_panel(bool *value, char *text, int is_tree, int filled, int align_right);
int ui_sub_image(gpu_texture_t *image, int tint, int h, int sx, int sy, int sw, int sh);
int ui_image(gpu_texture_t *image, int tint, int h);
char *ui_text_input(char* *value, char *label, int align, int editable, int live_update);
bool ui_check(bool *value, char *text, char *label);
bool ui_radio(int *value, int position, char *text, char *label);
int ui_combo(int *value, string_array_t *texts, char *label, int show_label, int align, int search_bar);
float ui_slider(float *value, char *text, float from, float to, int filled, float precision, int display_value, int align, int text_edit);
void ui_row(f32_array_t *ratios);
void ui_row2(void);
void ui_row3(void);
void ui_row4(void);
void ui_row5(void);
void ui_row6(void);
void ui_row7(void);
void ui_separator(int h, int fill);
void ui_tooltip(char *text);
void ui_tooltip_image(gpu_texture_t *image, int max_width);
void ui_end(void);
void ui_end_window(void);
void ui_mouse_down(ui_t *ui, int button, int x, int y);
void ui_mouse_move(ui_t *ui, int x, int y, int movement_x, int movement_y);
void ui_mouse_up(ui_t *ui, int button, int x, int y);
void ui_mouse_wheel(ui_t *ui, float delta);
void ui_key_down(ui_t *ui, int key_code);
void ui_key_up(ui_t *ui, int key_code);
void ui_key_press(ui_t *ui, int character);
void ui_set_scale(float factor);
bool ui_get_hover(float elem_h);
bool ui_get_released(float elem_h);
bool ui_input_in_rect(float x, float y, float w, float h);
void ui_fill(float x, float y, float w, float h, int color);
void ui_rect(float x, float y, float w, float h, int color, float strength);
bool ui_is_visible(float elem_h);
void ui_end_element(void);
void ui_end_element_of_size(float element_size);
void ui_fade_color(float alpha);
void ui_draw_string(char *text, float x_offset, float y_offset, int align, int truncation);
void ui_draw_shadow(float x, float y, float w, float h);
void ui_draw_rect(int fill, int shadows, float x, float y, float w, float h);
void ui_start_text_edit(char* *value, int align);
float UI_SCALE(void);
float UI_ELEMENT_W(void);
float UI_ELEMENT_H(void);
float UI_ELEMENT_OFFSET(void);
float UI_ARROW_SIZE(void);
float UI_BUTTON_H(void);
float UI_CHECK_SIZE(void);
float UI_CHECK_SELECT_SIZE(void);
float UI_FONT_SIZE(void);
float UI_SCROLL_W(void);
float UI_TEXT_OFFSET(void);
float UI_TAB_W(void);
float UI_HEADER_DRAG_H(void);
float UI_TOOLTIP_DELAY(void);
float ui_float_input(float *value, char *label, int align, float precision);
int ui_inline_radio(int *value, string_array_t *texts, int align);
int ui_color_wheel(int *value, ui_color_state_t *state, int alpha, float w, float h, int color_preview, void *picker, void *data);
char *ui_text_area(char* *value, int *line_index, int align, int editable, char *label, int word_wrap);
void ui_begin_menu(void);
void ui_end_menu(void);
bool ui_menubar_button(char *text);
int ui_color_r(int color);
int ui_color_g(int color);
int ui_color_b(int color);
int ui_color_a(int color);
int ui_color(int r, int g, int b, int a);
void ui_nodes_init(ui_nodes_t *nodes);
void ui_node_canvas(ui_nodes_t *nodes, ui_node_canvas_t *canvas);
void ui_nodes_rgba_popup(ui_color_state_t *state, float *val, int x, int y);
void ui_remove_node(ui_node_t *n, ui_node_canvas_t *canvas);
float UI_NODES_SCALE(void);
float UI_NODES_PAN_X(void);
float UI_NODES_PAN_Y(void);
float UI_NODE_X(ui_node_t *node);
float UI_NODE_Y(ui_node_t *node);
float UI_NODE_W(ui_node_t *node);
float UI_NODE_H(ui_node_canvas_t *canvas, ui_node_t *node);
float UI_OUTPUT_Y(ui_node_t *node, int pos);
float UI_INPUT_Y(ui_node_canvas_t *canvas, ui_node_t *node, int pos);
float UI_OUTPUTS_H(ui_node_t *node, int length);
float UI_BUTTONS_H(ui_node_t *node);
float UI_LINE_H(void);
int ui_get_socket_id(ui_node_array_t *nodes);
ui_node_link_t *ui_get_link(ui_node_link_array_t *links, int id);
int ui_next_link_id(ui_node_link_array_t *links);
ui_node_t *ui_get_node(ui_node_array_t *nodes, int id);
int ui_next_node_id(ui_node_array_t *nodes);
double sys_time(void);
double sys_delta(void);
double sys_real_delta(void);
int sys_w(void);
int sys_h(void);
int sys_x(void);
int sys_y(void);
char *sys_title(void);
void sys_title_set(char *value);
gpu_shader_t *sys_get_shader(char *name);
char *sys_buffer_to_string(buffer_t *b);
buffer_t *sys_string_to_buffer(char *str);
void line_draw_init(void);
void line_draw_lineb(int a, int b, int c, int d, int e, int f);
void line_draw_line(float x1, float y1, float z1, float x2, float y2, float z2);
void line_draw_begin(void);
void line_draw_end(void);
void draw_begin(gpu_texture_t *target, int clear, int color);
void draw_scaled_sub_image(gpu_texture_t *img, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);
void draw_scaled_image(gpu_texture_t *tex, float dx, float dy, float dw, float dh);
void draw_sub_image(gpu_texture_t *tex, float sx, float sy, float sw, float sh, float x, float y);
void draw_image(gpu_texture_t *tex, float x, float y);
void draw_filled_triangle(float x0, float y0, float x1, float y1, float x2, float y2);
void draw_filled_rect(float x, float y, float width, float height);
void draw_rect(float x, float y, float width, float height, float strength);
void draw_line(float x0, float y0, float x1, float y1, float strength);
void draw_line_aa(float x0, float y0, float x1, float y1, float strength);
void draw_string(char *text, float x, float y);
void draw_end(void);
void draw_flush(void);
void draw_set_color(int color);
int draw_get_color(void);
void draw_set_pipeline(gpu_pipeline_t *pipeline);
bool draw_set_font(draw_font_t *font, int size);
float draw_sub_string_width(draw_font_t *font, int font_size, char *text, int start, int end);
int draw_string_width(draw_font_t *font, int font_size, char *text);
void draw_filled_circle(float cx, float cy, float radius, int segments);
void draw_circle(float cx, float cy, float radius, int segments, float strength);
void draw_cubic_bezier(f32_array_t *x, f32_array_t *y, int segments, float strength);
void audio_play(iron_a1_sound_t *sound, int loop);
void audio_stop(iron_a1_sound_t *sound);
char *string_alloc(int size);
char *string_copy(char *a);
int string_length(char *str);
bool string_equals(char *a, char *b);
char *i32_to_string(int i);
char *i32_to_string_hex(int i);
char *i64_to_string(int i);
char *u64_to_string(int i);
char *f32_to_string(float f);
char *f32_to_string_with_zeros(float f);
void string_strip_trailing_zeros(char *str);
int string_index_of(char *s, char *search);
int string_index_of_pos(char *s, char *search, int pos);
int string_last_index_of(char *s, char *search);
any_array_t *string_split(char *s, char *sep);
char *string_array_join(any_array_t *a, char *separator);
char *string_replace_all(char *s, char *search, char *replace);
char *substring(char *s, int start, int end);
char *string_from_char_code(int c);
int char_code_at(char *s, int i);
char *char_at(char *s, int i);
bool starts_with(char *s, char *start);
bool ends_with(char *s, char *end);
char *to_lower_case(char *s);
char *to_upper_case(char *s);
char *trim_end(char *str);
int string_utf8_decode(char *str, int *i);
int parse_int(char *s);
float parse_float(char *s);
int iron_websocket_open(char *url);
void iron_websocket_send(int ws, char *text);
char *iron_websocket_receive(int ws);
int iron_websocket_state(int ws);
void iron_websocket_close(int ws);
bool iron_file_reader_open(iron_file_reader_t *reader, char *filepath, int type);
bool iron_file_reader_close(iron_file_reader_t *reader);
int iron_file_reader_read(iron_file_reader_t *reader, void *data, int size);
int iron_file_reader_size(iron_file_reader_t *reader);
int iron_file_reader_pos(iron_file_reader_t *reader);
bool iron_file_reader_seek(iron_file_reader_t *reader, int pos);
bool iron_file_writer_open(iron_file_writer_t *writer, char *filepath);
void iron_file_writer_write(iron_file_writer_t *writer, void *data, int size);
void iron_file_writer_close(iron_file_writer_t *writer);
char *iron_read_directory(char *path);
void iron_create_directory(char *path);
bool iron_is_directory(char *path);
bool iron_file_exists(char *path);
void iron_delete_file(char *path);
void iron_file_save_bytes(char *path, buffer_t *bytes, int length);
void iron_write_png(char *path, buffer_t *bytes, int w, int h, int format);
void iron_write_jpg(char *path, buffer_t *bytes, int w, int h, int format, int quality);
void iron_file_download(char *url, void *callback, int size, char *dst_path);
any_array_t *file_read_directory(char *path);
void file_copy(char *src_path, char *dst_path);
void file_start(char *path);
void file_download_to(char *url, char *dst_path, void *done, int size);
void *malloc(int size);
void *calloc(int n, int size);
void *realloc(void *ptr, int size);
void free(void *ptr);
void i32_map_set(i32_map_t *m, char *k, int v);
void f32_map_set(f32_map_t *m, char *k, float v);
void any_map_set(any_map_t *m, char *k, void *v);
int i32_map_get(i32_map_t *m, char *k);
float f32_map_get(f32_map_t *m, char *k);
void *any_map_get(any_map_t *m, char *k);
void map_delete(any_map_t *m, char *k);
any_array_t *map_keys(any_map_t *m);
i32_map_t *i32_map_create(void);
any_map_t *any_map_create(void);
void i32_imap_set(i32_imap_t *m, int k, int v);
void any_imap_set(any_imap_t *m, int k, void *v);
int i32_imap_get(i32_imap_t *m, int k);
void *any_imap_get(any_imap_t *m, int k);
void imap_delete(any_imap_t *m, int k);
i32_array_t *imap_keys(any_imap_t *m);
any_imap_t *any_imap_create(void);
void array_free(void *a);
void i8_array_push(i8_array_t *a, int e);
void u8_array_push(u8_array_t *a, int e);
void i16_array_push(i16_array_t *a, int e);
void u16_array_push(u16_array_t *a, int e);
void i32_array_push(i32_array_t *a, int e);
void u32_array_push(u32_array_t *a, int e);
void f32_array_push(f32_array_t *a, float e);
void any_array_push(any_array_t *a, void *e);
void string_array_push(string_array_t *a, void *e);
void i8_array_resize(i8_array_t *a, int size);
void u8_array_resize(u8_array_t *a, int size);
void i16_array_resize(i16_array_t *a, int size);
void u16_array_resize(u16_array_t *a, int size);
void i32_array_resize(i32_array_t *a, int size);
void u32_array_resize(u32_array_t *a, int size);
void f32_array_resize(f32_array_t *a, int size);
void any_array_resize(any_array_t *a, int size);
void string_array_resize(string_array_t *a, int size);
void buffer_resize(buffer_t *b, int size);
void array_sort(any_array_t *ar, void *compare);
void i32_array_sort(i32_array_t *ar, void *compare);
void *array_pop(any_array_t *ar);
int i32_array_pop(i32_array_t *ar);
void *array_shift(any_array_t *ar);
void array_splice(any_array_t *ar, int start, int delete_count);
void i32_array_splice(i32_array_t *ar, int start, int delete_count);
any_array_t *array_concat(any_array_t *a, any_array_t *b);
any_array_t *array_slice(any_array_t *a, int begin, int end);
void array_insert(any_array_t *a, int at, void *e);
void array_remove(any_array_t *ar, void *e);
void string_array_remove(string_array_t *ar, char *e);
void i32_array_remove(i32_array_t *ar, int e);
int array_index_of(any_array_t *ar, void *e);
int string_array_index_of(string_array_t *ar, char *e);
int i32_array_index_of(i32_array_t *ar, int e);
void array_reverse(any_array_t *ar);
buffer_t *buffer_slice(buffer_t *a, int begin, int end);
int buffer_get_u8(buffer_t *b, int p);
int buffer_get_i8(buffer_t *b, int p);
int buffer_get_u16(buffer_t *b, int p);
int buffer_get_i16(buffer_t *b, int p);
float buffer_get_f16(buffer_t *b, int p);
int buffer_get_u32(buffer_t *b, int p);
int buffer_get_i32(buffer_t *b, int p);
float buffer_get_f32(buffer_t *b, int p);
float buffer_get_f64(buffer_t *b, int p);
int buffer_get_i64(buffer_t *b, int p);
void buffer_set_u8(buffer_t *b, int p, int n);
void buffer_set_i8(buffer_t *b, int p, int n);
void buffer_set_u16(buffer_t *b, int p, int n);
void buffer_set_i16(buffer_t *b, int p, int n);
void buffer_set_u32(buffer_t *b, int p, int n);
void buffer_set_i32(buffer_t *b, int p, int n);
void buffer_set_f32(buffer_t *b, int p, float n);
buffer_t *buffer_create(int length);
buffer_t *buffer_create_from_raw(uint8_t *raw, int length);
f32_array_t *f32_array_create(int length);
f32_array_t *f32_array_create_from_buffer(buffer_t *b);
f32_array_t *f32_array_create_from_array(f32_array_t *from);
f32_array_t *f32_array_create_from_raw(float *raw, int length);
f32_array_t *f32_array_create_x(float x);
f32_array_t *f32_array_create_xy(float x, float y);
f32_array_t *f32_array_create_xyz(float x, float y, float z);
f32_array_t *f32_array_create_xyzw(float x, float y, float z, float w);
f32_array_t *f32_array_create_xyzwv(float x, float y, float z, float w, float v);
u32_array_t *u32_array_create(int length);
u32_array_t *u32_array_create_from_array(u32_array_t *from);
u32_array_t *u32_array_create_from_raw(uint32_t *raw, int length);
i32_array_t *i32_array_create(int length);
i32_array_t *i32_array_create_from_array(i32_array_t *from);
i32_array_t *i32_array_create_from_raw(int32_t *raw, int length);
u16_array_t *u16_array_create(int length);
u16_array_t *u16_array_create_from_raw(uint16_t *raw, int length);
i16_array_t *i16_array_create(int length);
i16_array_t *i16_array_create_from_array(i16_array_t *from);
i16_array_t *i16_array_create_from_raw(int16_t *raw, int length);
u8_array_t *u8_array_create(int length);
u8_array_t *u8_array_create_from_array(u8_array_t *from);
u8_array_t *u8_array_create_from_raw(uint8_t *raw, int length);
u8_array_t *u8_array_create_from_string(char *s);
char *u8_array_to_string(u8_array_t *a);
i8_array_t *i8_array_create(int length);
i8_array_t *i8_array_create_from_raw(int8_t *raw, int length);
any_array_t *any_array_create(int length);
any_array_t *any_array_create_from_raw(void *raw, int length);
string_array_t *string_array_create(int length);
int float_to_half_fast(float value);
int half_to_u8_fast(int h);
bool mouse_down(char *button);
bool mouse_down_any(void);
bool mouse_started(char *button);
bool mouse_started_any(void);
bool mouse_released(char *button);
float mouse_view_x(void);
float mouse_view_y(void);
void iron_mouse_lock(void);
void iron_mouse_unlock(void);
bool iron_mouse_is_locked(void);
bool keyboard_down(char *key);
bool keyboard_started(char *key);
bool keyboard_started_any(void);
bool keyboard_released(char *key);
bool keyboard_repeat(char *key);
char *keyboard_key_code(int key);
plugin_t *plugin_create(void);
void plugin_notify_on_ui(plugin_t *plugin, void *f);
void plugin_notify_on_update(plugin_t *plugin, void *f);
void plugin_notify_on_delete(plugin_t *plugin, void *f);
int plugin_register_tool(plugin_t *plugin, char *name, char *tooltip, int icon, void *on_header, void *on_stroke, void *on_stroke_end);
void plugin_tool_set_overlay(int tool, gpu_texture_t *image, int color);
void script_notify_on_update(void *fn);
void script_notify_on_next_frame(void *fn);
void console_info(char *s);
void console_error(char *s);
void console_log(char *s);
void ui_box_show_message(char *title, char *text, int copyable);
void ui_files_show2(char *filters, int is_save, int open_multiple, void *files_done);
void ui_box_show_custom2(void *commands, int w, int h, void *on_hide, char *title);
void ui_box_hide(void);
void script_quit(void);
void base_redraw_ui(void);
void project_save(int save_and_quit);
void script_project_new(void);
void script_project_open(char *path);
void script_import_asset(char *path, int hdr_as_envmap);
int script_asset_index(char *search);
void script_append_mesh(char *path);
void script_append_mesh_skinned(char *glb_path);
buffer_t *script_load_blob(char *path);
void script_append_mesh_obj(char *data);
void script_export_mesh(char *path);
void script_export_material(char *path);
void util_mesh_decimate(any_array_t *objects, float ratio);
void util_mesh_smooth(any_array_t *objects);
void util_mesh_bevel(any_array_t *objects, float amount);
void util_mesh_subdivide(any_array_t *objects);
void util_mesh_merge_geometry(void);
void util_mesh_duplicate(void);
void util_mesh_visibility_changed(void);
char *project_filepath_get(void);
char *project_basepath_get(void);
void project_filepath_set(char *s);
camera_object_t *script_get_camera(void);
context_t *script_get_context(void);
config_t *script_get_config(void);
project_t *script_get_project(void);
world_data_t *script_get_world(void);
object_t *script_get_object(char *s);
slot_material_t *script_get_material(char *s);
sound_t *script_get_sound(char *s);
int script_sound_remove(char *s);
gpu_texture_t *script_get_texture(char *s);
buffer_t *script_get_texture_pixels(gpu_texture_t *texture);
void script_texture_delete(char *s);
string_array_t *script_shape_list(void);
object_t *script_shape_add(char *name);
object_t *script_object_duplicate(object_t *o);
object_t *script_object_clone(char *name);
void script_object_remove(object_t *o);
void script_object_set_name(object_t *object, char *name);
void script_object_origin_to_geometry(object_t *o);
void script_physics_set_shape(object_t *o, int shape);
void script_physics_set_mass(object_t *o, float mass);
void script_physics_apply_impulse(object_t *o, float x, float y, float z);
void script_physics_set_velocity(object_t *o, float x, float y, float z);
void script_physics_sync_transform(object_t *o);
physics_body_t *script_physics_get_body(object_t *o);
object_t *script_pick_object(void);
void script_mouse_click(int x, int y, int button);
void script_key_press(char *key);
any_array_t *script_get_mesh_objects(void);
void script_call(void *fn);
void script_show_message(char *text, float seconds);
void script_set_stage(char *name);
void script_stage_create(char *name);
void script_stage_add_object(char *stage, char *object);
void script_stage_remove_object(char *stage, char *object);
void script_stage_set_hidden(char *stage, char *object, int hidden);
void script_show_envmap(int b);
void script_set_tools(int tools);
void script_fade_to_stage(char *stage);
void script_timeline_resume(void);
void script_timeline_pause(void);
int script_timeline_get_frame(void);
void script_timeline_set_frame(int frame);
void script_clip_play(object_t *o, int start, int end);
void script_clip_set_frame(object_t *o, int frame);
int script_clip_frame_count(object_t *o);
int tab_timeline_get_frame_rate(void);
void script_timeline_add_keyframe(char *name, int frame);
void script_timer(float delay, void *fn);
void script_cloud_init(void *done);
void script_cloud_download(char *path, void *done);
void script_screenshot(char *path);
char *script_get_stage(void);
void script_set_tilesheet_anim(object_t *o, char *anim);
void script_draw_particles(gpu_texture_t *texture, float x, float y, float w, float h, int atlas_x, int atlas_frames);
void script_paint(float x, float y);
void script_paint_world(float x, float y, float z);
void script_paint_end(void);
void script_fill_layer(void);
slot_layer_t *script_layer_get(char *name);
void script_layer_set(slot_layer_t *l);
slot_layer_t *script_layer_new(void);
slot_layer_t *script_layer_new_fill(void);
slot_layer_t *script_layer_new_color(int base_color, float occlusion, float roughness, float metallic);
void script_layer_delete(slot_layer_t *l);
int script_layer_count(void);
slot_layer_t *script_layer_at(int index_from_bottom);
int script_layer_index(slot_layer_t *l);
slot_layer_t *script_layer_new_mask(slot_layer_t *l, int black0_white1_fill2);
slot_layer_t *script_layer_new_group(slot_layer_t *l);
bool script_layer_move(slot_layer_t *l, int to_index_from_bottom);
bool script_layer_set_group(slot_layer_t *l, slot_layer_t *group);
void script_layer_set_object(slot_layer_t *l, object_t *object);
void script_layer_set_name(slot_layer_t *l, char *name);
void script_layer_update(slot_layer_t *l);
slot_layer_t *script_layer_duplicate(slot_layer_t *l);
slot_layer_t *script_layer_merge_down(slot_layer_t *l);
slot_layer_t *script_layer_merge_group(slot_layer_t *group);
void script_layer_clear(slot_layer_t *l);
bool script_layer_invert_mask(slot_layer_t *m);
slot_layer_t *script_layer_apply_mask(slot_layer_t *m);
bool script_layer_to_fill(slot_layer_t *l);
bool script_layer_to_paint(slot_layer_t *l);
slot_layer_t *script_layer_new_path(bool curved);
slot_layer_t *script_layer_new_text(char *text);
bool script_layer_add_path_point(slot_layer_t *l, float x, float y);
void script_layer_repaint_path(slot_layer_t *l);
void script_bake_lightmap(object_t *o, int res, int samples, float range, char *path, void *done);
bool script_bake_lightmap_busy(void);
void script_bake_lightmap_cancel(void);
slot_material_t *script_material_create(char *name);
void script_material_set(slot_material_t *m);
void script_object_set_material(object_t *object, slot_material_t *material);
void script_material_delete(slot_material_t *m);
ui_node_t *script_material_create_node(char *type);
ui_node_t *script_material_create_node_at(char *type, float x, float y);
ui_node_t *script_material_get_node(char *type);
ui_node_t *script_material_get_node_id(int id);
void script_material_connect(ui_node_t *from, int from_socket, ui_node_t *to, int to_socket);
void script_material_disconnect(ui_node_t *to, int to_socket);
void script_material_remove_node(ui_node_t *node);
void script_material_add_input(ui_node_t *node, char *type, char *name);
void script_material_add_output(ui_node_t *node, char *type, char *name);
void script_material_set_float(ui_node_t *node, int is_input, int socket, float value);
void script_material_set_color(ui_node_t *node, int is_input, int socket, float r, float g, float b, float a);
void script_material_set_vector(ui_node_t *node, int is_input, int socket, float x, float y, float z);
void script_material_set_button(ui_node_t *node, int button, float value);
void script_material_set_text(ui_node_t *node, int button, char *text);
void script_material_update(void);
ui_node_t *script_mesh_create_node_at(mesh_object_t *o, char *type, float x, float y);
ui_node_t *script_mesh_get_node(mesh_object_t *o, char *type);
void script_mesh_connect(mesh_object_t *o, ui_node_t *from, int from_socket, ui_node_t *to, int to_socket);
void script_mesh_disconnect(mesh_object_t *o, ui_node_t *to, int to_socket);
void script_mesh_remove_node(mesh_object_t *o, ui_node_t *node);
void script_mesh_add_input(mesh_object_t *o, ui_node_t *node, char *type, char *name);
void script_mesh_update(mesh_object_t *o);
mesh_geo_t *mesh_geo_input(void);
void mesh_geo_output(mesh_geo_t *g);
mesh_geo_t *mesh_geo_create(int vertex_count, int index_count);
mesh_geo_t *mesh_geo_copy(mesh_geo_t *g);
mesh_geo_t *mesh_geo_shape(int shape, float size_x, float size_y, float size_z, int segments);
mesh_geo_t *mesh_geo_transform(mesh_geo_t *g, float loc_x, float loc_y, float loc_z, float rot_x, float rot_y, float rot_z, float scale_x, float scale_y, float scale_z);
mesh_geo_t *mesh_geo_array(mesh_geo_t *g, int count, float offset_x, float offset_y, float offset_z, bool relative);
mesh_geo_t *mesh_geo_mirror(mesh_geo_t *g, int axis);
mesh_geo_t *mesh_geo_join(mesh_geo_t *a, mesh_geo_t *b);
bool mesh_geo_append(mesh_geo_t *dst, mesh_geo_t *src, float *m);
mesh_geo_t *mesh_geo_boolean(mesh_geo_t *a, mesh_geo_t *b, int op);
mesh_geo_t *mesh_geo_subdivide(mesh_geo_t *g, int levels);
mesh_geo_t *mesh_geo_smooth(mesh_geo_t *g, int iterations);
mesh_geo_t *mesh_geo_decimate(mesh_geo_t *g, float ratio);
mesh_geo_t *mesh_geo_bevel(mesh_geo_t *g, float amount);
mesh_geo_t *mesh_geo_normals(mesh_geo_t *g, bool smooth);
float mesh_geo_input_value(char *name);
float mesh_geo_input_vector(char *name, int i);
mesh_geo_t *mesh_geo_input_mesh(char *name);
mesh_geo_image_t *mesh_geo_input_image(char *name);
float mesh_geo_image_sample(mesh_geo_image_t *img, float u, float v, int channel);
void *mesh_geo_state(int size);
void context_set_viewport_shader(void *viewport_shader);
render_pass_t *render_pass_create(char *frag, int stage);
void render_pass_set_vec4(render_pass_t *p, char *name, float x, float y, float z, float w);
void render_pass_remove(render_pass_t *p);
void context_set_viewport_texture(gpu_texture_t *image);
void context_set_viewport_mode(int mode);
void context_set_camera_controls(int i);
void node_shader_write_frag(node_shader_t *raw, char *s);
void script_register_texture(char *format, void *fn);
void script_unregister_texture(char *format);
void script_register_mesh(char *format, void *fn);
void script_unregister_mesh(char *format);
void util_mesh_uv_unwrap(any_array_t *objects);
void geom_sdf_begin(float voxel);
void geom_sdf_transform(int axis, float deg, float x, float y, float z);
void geom_sdf_box(float x0, float y0, float z0, float x1, float y1, float z1, float round, int op);
void geom_sdf_cylinder(float x, float y, float z, float r, float h, int axis, float round, int op);
void geom_sdf_ellipsoid(float x, float y, float z, float rx, float ry, float rz, int op);
void geom_sdf_capsule(float ax, float ay, float az, float bx, float by, float bz, float ra, float rb, int op);
void geom_sdf_torus(float x, float y, float z, float major, float minor, float zscale, int op);
void geom_sdf_zigzag(float side, float amp, int count, int op);
geom_t *geom_sdf_end(int smooth);
geom_t *geom_box(float x0, float y0, float z0, float x1, float y1, float z1);
geom_t *geom_grid(float x0, float y0, float x1, float y1, int nx, int ny);
geom_t *geom_cylinder(float x, float y, float z, float r, float h, int segs);
geom_t *geom_torus(float x, float y, float z, float major, float minor, int segs, int sides);
void geom_rotate(geom_t *g, int axis, float deg, float px, float py, float pz);
void geom_translate(geom_t *g, float x, float y, float z);
void geom_flip(geom_t *g);
geom_t *geom_copy(geom_t *g);
void geom_keep(geom_t *g, int axis, float lo, float hi);
void geom_clay(geom_t *g, float amp, float freq, float seed, float fine);
void geom_unwrap(geom_t *g, float margin);
void geom_export_add(geom_t *g, char *name, int flat);
void geom_export_write_obj(char *path);
geom_t *geom_ellipsoid(float x, float y, float z, float rx, float ry, float rz, int segs, int rings);
void geom_basis(geom_t *g, float ux, float uy, float uz, float wx, float wy, float wz, float px, float py, float pz);
void geom_scale(geom_t *g, float s);
void geom_tube_begin(void);
void geom_tube_point(float x, float y, float z, float r);
geom_t *geom_tube_end(int sides, int steps, float squash, int curve_steps, int transport);
int geom_vertex_count(geom_t *g);
float geom_vertex_get(geom_t *g, int v, int axis);
void geom_vertex_set(geom_t *g, int v, float x, float y, float z);
void geom_weights_blend(geom_t *g, int a, int b, float ax, float ay, float az, float bx, float by, float bz);
void geom_weights(geom_t *g, int v, int b0, float w0, int b1, float w1, int b2, float w2);
void geom_part(geom_t *g, int part);
geom_t *geom_merge(geom_t *dst, geom_t *src);
void geom_displace(geom_t *g, float freq0, float amp0, float freq1, float amp1);
void geom_palette(int part, int r, int g, int b);
void geom_atlas(geom_t *g, int size, int seed, char *path);
void skel_begin(void);
int skel_bone(int parent, float hx, float hy, float hz, float tx, float ty, float tz, float zx, float zy, float zz);
void skel_clip_begin(void);
void skel_pose(int bone, float rx, float ry, float rz, float lx, float ly, float lz);
void skel_frame(void);
bool skel_write(geom_t *g, char *name, char *path);
void env_begin(int w, int h, float r, float g, float b);
void env_add_lobe(float dx, float dy, float dz, float power, float r, float g, float b);
void env_add_disc(float dx, float dy, float dz, float deg, float irradiance, float r, float g, float b);
void env_write(char *path);
bool script_packed_asset_save(char *suffix, char *path);
int script_packed_assets_remove(char *search);
int script_assets_remove(char *search);
int script_pack_assets(char *search);
void script_set_pack_assets(int pack);
any_array_t *util_mesh_get_hierarchy(mesh_object_t *o);
void script_register_text(char *format, void *fn);
void script_unregister_text(char *format);
raw_mesh_t *script_make_raw_mesh(char *name, i16_array_t *posa, i16_array_t *nora, u32_array_t *inda, float scale_pos);
void script_material_category_add(char *category_name, any_array_t *node_list);
void script_brush_category_add(char *category_name, any_array_t *node_list);
void script_material_category_remove(char *category_name);
void script_brush_category_remove(char *category_name);
void script_material_custom_nodes_set(char *node_type, void *fn);
void script_brush_custom_nodes_set(char *node_type, void *fn);
void script_material_custom_nodes_remove(char *node_type);
void script_brush_custom_nodes_remove(char *node_type);
void *script_material_kong_get(void);
char *parser_material_parse_value_input(ui_node_socket_t *inp, int vector_as_grayscale);
mesh_object_t *context_main_object(void);
void export_texture_run(char *path, int bake_material);
void context_select_tool(int i);
gpu_texture_t *gpu_create_render_target(int width, int height, int format);
void viewport_capture_screenshot_to(gpu_texture_t *target, float x, float y, float w, float h);
void viewport_save_texture(gpu_texture_t *screenshot);
void viewport_save_texture_to_file(gpu_texture_t *screenshot, char *path);
bool project_reskin_mesh(int frame);
void iron_delay_idle_sleep(void);
void iron_load_url(char *url);
void *json_parse(char *s);
any_map_t *json_parse_to_map(char *s);
void json_encode_begin(void);
char *json_encode_end(void);
void json_encode_string(char *k, char *v);
void json_encode_string_array(char *k, string_array_t *a);
void json_encode_f32(char *k, float f);
void json_encode_i32(char *k, int i);
void json_encode_null(char *k);
void json_encode_f32_array(char *k, f32_array_t *a);
void json_encode_i32_array(char *k, i32_array_t *a);
void json_encode_bool(char *k, int b);
void json_encode_begin_array(char *k);
void json_encode_end_array(void);
void json_encode_begin_object(void);
void json_encode_begin_object_key(char *k);
void json_encode_end_object(void);
void json_encode_map(any_map_t *m);
buffer_t *json_encode_to_armpack(char *json);
void *armpack_decode(buffer_t *b);
any_map_t *armpack_decode_to_map(buffer_t *b);
char *armpack_decode_to_json(buffer_t *b);
void armpack_encode_start(void *encoded);
int armpack_encode_end(void);
void armpack_encode_map(int count);
void armpack_encode_array(int count);
void armpack_encode_array_f32(f32_array_t *f32a);
void armpack_encode_array_i32(i32_array_t *i32a);
void armpack_encode_array_i16(i16_array_t *i16a);
void armpack_encode_array_u8(u8_array_t *u8a);
void armpack_encode_array_string(string_array_t *strings);
void armpack_encode_string(char *str);
void armpack_encode_i32(int i);
void armpack_encode_f32(float f);
void armpack_encode_bool(int b);
void armpack_encode_null(void);
int armpack_size_map(void);
int armpack_size_array(void);
int armpack_size_array_f32(f32_array_t *f32a);
int armpack_size_array_u8(u8_array_t *u8a);
int armpack_size_string(char *str);
int armpack_size_i32(void);
int armpack_size_f32(void);
int armpack_size_bool(void);
float armpack_map_get_f32(any_map_t *map, char *key);
int armpack_map_get_i32(any_map_t *map, char *key);

// Material nodes reference:
// ATTRIBUTE | out: 0 Color, 1 Vector, 2 Factor, 3 Alpha
//     button 0 Name STRING
// RGB | out: 0 Color
//     button 0 RGBA RGBA
// NEW_GEOMETRY | out: 0 Position, 1 Normal, 2 Tangent, 3 Incoming, 4 Parametric, 5 Pointiness
// LAYER | out: 0 Base Color, 1 Opacity, 2 Occlusion, 3 Roughness, 4 Metallic, 5 Normal Map, 6 Emission, 7 Height, 8 Subsurface
//     button 0 Layer ENUM: 0 
// LAYER_MASK | out: 0 Value
//     button 0 Layer ENUM: 0 
// MATERIAL | out: 0 Base Color, 1 Opacity, 2 Occlusion, 3 Roughness, 4 Metallic, 5 Normal Map, 6 Emission, 7 Height, 8 Subsurface
//     button 0 Material ENUM: 0 
// OBJECT_INFO | out: 0 Location, 1 Object Index, 2 Material Index, 3 Random
// PICKER | out: 0 Base Color, 1 Opacity, 2 Occlusion, 3 Roughness, 4 Metallic, 5 Normal Map, 6 Emission, 7 Height, 8 Subsurface
// SCRIPT_CPU | out: 0 Value
//     button 0 script_node_button CUSTOM
// SHADER_GPU | out: 0 Color
//     button 0 shader_node_button CUSTOM
//     button 1 shader_node_add_button CUSTOM
//     Constants: float constants.time, float constants.frame, float4 constants.path_a, float4 constants.path_b, float constants.path_length
//     Path layers paint the material one segment at a time: path_a.xyz and path_b.xyz are the segment ends in world
//     space like, .w their arc length along the path, path_length the arc length of the path.
//     Inputs: script_material_add_input(sh, "VALUE", "Bark Scale") adds a VALUE, VECTOR or RGBA input socket.
//     The shader text reads it as a variable named after the socket, in lower case with other characters
//     replaced by _: float bark_scale. VECTOR and RGBA are float3.
//     Outputs: the text writes each output socket as output.<name>, named like inputs: output.color is float3,
//     a VALUE output is float. Outputs not written are 0. The text runs once for all outputs, return output; exits
//     early. The node starts with a Color output, script_material_add_output(sh, "VALUE", "Height") adds a VALUE,
//     VECTOR or RGBA output socket.
//     Example:
//     ui_node_t *sh = script_material_create_node_at("SHADER_GPU", -400.0, 0.0);
//     script_material_set_text(sh, 0, "float2 c = frac(tex_coord * 8.0);\nfloat d = length(c - float2(0.5, 0.5));\noutput.color = lerp(float3(1.0, 0.8, 0.2), float3(0.1, 0.1, 0.3), smoothstep(dot_size - 0.02, dot_size, d));\noutput.height = 1.0 - smoothstep(0.0, dot_size, d);");
//     script_material_add_input(sh, "VALUE", "Dot Size");
//     script_material_set_float(sh, 1, 0, 0.3);
//     script_material_add_output(sh, "VALUE", "Height");
//     ui_node_t *out = script_material_get_node("OUTPUT_MATERIAL_PBR");
//     script_material_connect(sh, 0, out, 0); // Base Color
//     script_material_connect(sh, 1, out, 7); // Height
//     script_material_update();
// TEX_COORD | out: 0 Generated, 1 Normal, 2 UV, 3 Object, 4 Camera, 5 Window
// TILESHEET_ANIM | out: 0 UV
//     button 0 Tiles X VALUE
//     button 1 Tiles Y VALUE
//     button 2 Framerate VALUE
//     button 3 tilesheet_animation_node_button CUSTOM
//     button 4 Animation ENUM: 0 none
// TILESHEET | in: 0 Tile X, 1 Tile Y | out: 0 UV
//     button 0 Tiles X VALUE
//     button 1 Tiles Y VALUE
// UVMAP | out: 0 UV
//     button 0 UV Map ENUM: 0 uv0, 1 uv1
// VALUE | out: 0 Value
//     button 0 Value VALUE
// WIREFRAME | in: 0 Size | out: 0 Factor
//     button 0 Pixel Size BOOL
// TEX_BAKE | out: 0 Color
//     button 0 bake_texture_node_button CUSTOM
// TEX_BRICK | in: 0 Vector, 1 Color 1, 2 Color 2, 3 Mortar, 4 Scale, 5 Mortar Size, 6 Mortar Smooth, 7 Bias, 8 Brick Width, 9 Row Height, 10 Offset, 11 Frequency, 12 Squash, 13 Frequency | out: 0 Color, 1 Factor
// TEX_CAMERA | out: 0 Color
// TEX_CHECKER | in: 0 Vector, 1 Color 1, 2 Color 2, 3 Scale | out: 0 Color, 1 Factor
// BAKE_CURVATURE | in: 0 Strength, 1 Radius, 2 Offset | out: 0 Value
//     button 0 Convex Only BOOL
// TEX_GABOR | in: 0 Vector, 1 Scale, 2 Frequency, 3 Anisotropy, 4 Orientation 3D, 5 Orientation 2D | out: 0 Value, 1 Phase, 2 Intensity
//     button 0 Dimensions ENUM: 0 2D, 1 3D
// TEX_GRADIENT | in: 0 Vector | out: 0 Color, 1 Factor
//     button 0 gradient_type ENUM: 0 Linear, 1 Diagonal, 2 Radial, 3 Spherical
// TEX_IMAGE | in: 0 Vector | out: 0 Color, 1 Alpha
//     button 0 File ENUM: 0 
//     button 1 Color Space ENUM: 0 Auto, 1 Linear, 2 sRGB, 3 DirectX Normal Map
// TEX_MAGIC | in: 0 Vector, 1 Scale, 2 Distortion | out: 0 Color, 1 Factor
//     button 0 Depth VALUE
// TEX_NOISE | in: 0 Vector, 1 Scale, 2 Detail, 3 Roughness, 4 Lacunarity, 5 Distortion | out: 0 Factor, 1 Color
//     button 0 Dimensions ENUM: 0 3D
//     button 1 Type ENUM: 0 fBM
//     button 2 Normalize BOOL
// TEX_TEXT | in: 0 Vector | out: 0 Color, 1 Alpha
//     button 0 text STRING
// TEX_VORONOI | in: 0 Vector, 1 Scale, 2 Detail, 3 Roughness, 4 Lacunarity, 5 Randomness | out: 0 Distance, 1 Color, 2 Position
//     button 0 Dimensions ENUM: 0 2D, 1 3D
//     button 1 Feature Output ENUM: 0 F1, 1 F2
//     button 2 Normalize BOOL
// TEX_WAVE | in: 0 Vector, 1 Scale, 2 Distortion, 3 Detail Scale, 4 Detail Roughness, 5 Phase Offset | out: 0 Color, 1 Factor
//     button 0 Wave Type ENUM: 0 Bands, 1 Rings
//     button 1 Direction ENUM: 0 X, 1 Y, 2 Z, 3 Diagonal
//     button 2 Wave Profile ENUM: 0 Sine, 1 Saw, 2 Triangle
// BLUR | in: 0 Color, 1 Opacity, 2 Strength | out: 0 Color, 1 Opacity
//     button 0 Seamless BOOL
// BRIGHTCONTRAST | in: 0 Color, 1 Brightness, 2 Contrast | out: 0 Color
// COLMASK | in: 0 Color, 1 Mask Color, 2 Radius, 3 Fuzziness | out: 0 Mask
// VALTORGB | in: 0 Factor | out: 0 Color, 1 Alpha
//     button 0 nodes_material_color_ramp_button CUSTOM
// COMBINE_COLOR | in: 0 R, 1 G, 2 B | out: 0 Color
// GAMMA | in: 0 Color, 1 Gamma | out: 0 Color
// HUE_SAT | in: 0 Hue, 1 Saturation, 2 Value, 3 Factor, 4 Color | out: 0 Color
// INVERT_COLOR | in: 0 Factor, 1 Color | out: 0 Color
// MIX_RGB | in: 0 Factor, 1 Color 1, 2 Color 2 | out: 0 Color
//     button 0 blend_type ENUM: 0 Mix, 1 Darken, 2 Multiply, 3 Burn, 4 Lighten, 5 Screen, 6 Dodge, 7 Add, 8 Overlay, 9 Soft Light, 10 Linear Light, 11 Difference, 12 Exclusion, 13 Subtract, 14 Divide, 15 Hue, 16 Saturation, 17 Color, 18 Value
//     button 1 Clamp Factor BOOL
//     button 2 Clamp Result BOOL
// QUANTIZE | in: 0 Strength, 1 Color | out: 0 Color
// CURVE_RGB | in: 0 Fac, 1 Color | out: 0 Color
//     button 0 nodes_material_rgb_curves_button CUSTOM
// RGBTOBW | in: 0 Color | out: 0 Val
// REPLACECOL | in: 0 Color, 1 Old Color, 2 New Color, 3 Radius, 4 Fuzziness | out: 0 Color
// SEPARATE_COLOR | in: 0 Color | out: 0 R, 1 G, 2 B
// DIRECT_WARP | in: 0 Color, 1 Angle, 2 Mask | out: 0 Color
// BUMP | in: 0 Strength, 1 Distance, 2 Height, 3 Normal | out: 0 Normal Map
//     button 0 Invert BOOL
// CLAMP | in: 0 Value, 1 Min, 2 Max | out: 0 Value
//     button 0 operation ENUM: 0 Min Max, 1 Range
// COMBXYZ | in: 0 X, 1 Y, 2 Z | out: 0 Vector
// FLOAT_CURVE | in: 0 Factor, 1 Value | out: 0 Value
//     button 0 nodes_material_float_curve_button CUSTOM
// MAPRANGE | in: 0 Value, 1 From Min, 2 From Max, 3 To Min, 4 To Max | out: 0 Value
//     button 0 Clamp BOOL
// MAPPING | in: 0 Vector, 1 Location, 2 Rotation, 3 Scale | out: 0 Vector
// MATH | in: 0 Value, 1 Value | out: 0 Value
//     button 0 operation ENUM: 0 Add, 1 Subtract, 2 Multiply, 3 Divide, 4 Power, 5 Logarithm, 6 Square Root, 7 Inverse Square Root, 8 Absolute, 9 Exponent, 10 Minimum, 11 Maximum, 12 Less Than, 13 Greater Than, 14 Sign, 15 Round, 16 Floor, 17 Ceil, 18 Truncate, 19 Fraction, 20 Truncated Modulo, 21 Floored Modulo, 22 Snap, 23 Ping-Pong, 24 Sine, 25 Cosine, 26 Tangent, 27 Arcsine, 28 Arccosine, 29 Arctangent, 30 Arctan2, 31 Hyperbolic Sine, 32 Hyperbolic Cosine, 33 Hyperbolic Tangent, 34 To Radians, 35 To Degrees
//     button 1 Clamp BOOL
// MIX_NORMAL_MAP | in: 0 Normal Map 1, 1 Normal Map 2 | out: 0 Normal Map
//     button 0 blend_type ENUM: 0 Partial Derivative, 1 Whiteout, 2 Reoriented
// NORMAL | in: 0 Normal | out: 0 Normal, 1 Dot
//     button 0 Vector VECTOR
// NORMAL_MAP | in: 0 Strength, 1 Normal Map | out: 0 Normal Map
// SEPXYZ | in: 0 Vector | out: 0 X, 1 Y, 2 Z
// CURVE_VEC | in: 0 Factor, 1 Vector | out: 0 Vector
//     button 0 nodes_material_vector_curves_button CUSTOM
// VECT_MATH | in: 0 Vector, 1 Vector | out: 0 Vector, 1 Value
//     button 0 operation ENUM: 0 Add, 1 Subtract, 2 Multiply, 3 Divide, 4 Average, 5 Cross Product, 6 Project, 7 Reflect, 8 Dot Product, 9 Distance, 10 Length, 11 Scale, 12 Normalize, 13 Absolute, 14 Power, 15 Sign, 16 Minimum, 17 Maximum, 18 Floor, 19 Ceil, 20 Fraction, 21 Modulo, 22 Snap, 23 Sine, 24 Cosine, 25 Tangent
// VECT_ROTATE | in: 0 Vector, 1 Center, 2 Axis, 3 Angle | out: 0 Vector
//     button 0 Invert BOOL
// VECT_TRANSFORM | in: 0 Vector | out: 0 Vector
//     button 0 type ENUM: 0 Vector, 1 Point, 2 Normal
//     button 1 Convert From ENUM: 0 World, 1 Object, 2 Camera
//     button 2 Convert To ENUM: 0 World, 1 Object, 2 Camera
// NEURAL_EDIT_IMAGE | in: 0 Color, 1 Mask | out: 0 Color
//     button 0 edit_image_node_button CUSTOM
// NEURAL_IMAGE_TO_PBR | in: 0 Color | out: 0 Base Color, 1 Occlusion, 2 Roughness, 3 Normal Map, 4 Height
//     button 0 image_to_pbr_node_button CUSTOM
//     button 1 Tile BOOL
// NEURAL_REPEAT | out: 0 Out
//     button 0 repeat_node_button CUSTOM
// NEURAL_SAVE_IMAGE | in: 0 Color
// NEURAL_TEXT_TO_IMAGE | in: 0 In | out: 0 Color
//     button 0 text_to_image_node_button CUSTOM
//     button 1 Tile BOOL
// NEURAL_UPSCALE_IMAGE | in: 0 Color | out: 0 Color
//     button 0 upscale_image_node_button CUSTOM
//     button 1 Tile BOOL
// GROUP
//     button 0 nodes_material_new_group_button CUSTOM
//
// Pre-created material output node:
// OUTPUT_MATERIAL_PBR | in: 0 Base Color, 1 Opacity, 2 Occlusion, 3 Roughness, 4 Metallic, 5 Normal Map, 6 Emission, 7 Height, 8 Subsurface
//
// Mesh nodes reference:
// MESH_INPUT | out: 0 Mesh
// MESH_SHAPE | in: 0 Size, 1 Segments | out: 0 Mesh
//     button 0 Shape ENUM: 0 Box, 1 Sphere, 2 Cylinder, 3 Cone, 4 Torus, 5 Plane
// MESH_OBJECT | out: 0 Mesh
//     button 0 Mesh ENUM
// TEX_IMAGE | out: 0 Color, 1 Alpha
//     button 0 File ENUM: 0 
// LAYER_MASK | out: 0 Value
//     button 0 Layer ENUM: 0 
// MESH_TRANSFORM | in: 0 Mesh, 1 Location, 2 Rotation, 3 Scale | out: 0 Mesh
// MESH_ARRAY | in: 0 Mesh, 1 Count, 2 Offset | out: 0 Mesh
//     button 0 Relative Offset BOOL
// MESH_MIRROR | in: 0 Mesh | out: 0 Mesh
//     button 0 Axis ENUM: 0 X, 1 Y, 2 Z
// MESH_SUBDIVIDE | in: 0 Mesh, 1 Levels | out: 0 Mesh
// MESH_SMOOTH | in: 0 Mesh, 1 Iterations | out: 0 Mesh
// MESH_DECIMATE | in: 0 Mesh, 1 Ratio | out: 0 Mesh
// MESH_BEVEL | in: 0 Mesh, 1 Amount | out: 0 Mesh
// MESH_DISPLACE | in: 0 Mesh, 1 Strength | out: 0 Mesh
//     button 0 Layer ENUM: 0 
// MESH_SCRIPT | in: 0 Mesh | out: 0 Mesh
//     button 0 nodes_mesh_script_button CUSTOM
//     button 1 nodes_mesh_script_add_button CUSTOM
// MESH_JOIN | in: 0 Mesh, 1 Mesh | out: 0 Mesh
// MESH_BOOLEAN | in: 0 Mesh, 1 Mesh | out: 0 Mesh
//     button 0 Operation ENUM: 0 Union, 1 Difference, 2 Intersect
// MESH_VALUE | in: 0 Value | out: 0 Value
// MESH_MATH | in: 0 Value, 1 Value | out: 0 Value
//     button 0 Operation ENUM: 0 Add, 1 Subtract, 2 Multiply, 3 Divide, 4 Power, 5 Minimum, 6 Maximum, 7 Modulo, 8 Sine, 9 Cosine, 10 Absolute, 11 Round, 12 Floor
// MESH_VECTOR | in: 0 X, 1 Y, 2 Z | out: 0 Vector
// MESH_OUTPUT | in: 0 Mesh
//
// Mesh nodes build the mesh of an object from its original mesh, each object has its own canvas with a
// pre-created MESH_INPUT (original mesh) and MESH_OUTPUT node. Build it with script_mesh_create_node_at(),
// script_mesh_get_node(), script_mesh_connect() and apply with script_mesh_update(). Node values are set with
// script_material_set_float/set_vector/set_button/set_text, which work on any node.
//
// MESH_SCRIPT runs a minic script on the mesh linked to its input, the script text is button 0:
//     mesh_geo_t *g = mesh_geo_input(); // Editable copy of the input, float g->pos[vertex_count * 3],
//                                       // g->nor[vertex_count * 3], g->tex[vertex_count * 2], int g->ind[index_count]
//     mesh_geo_output(g);               // Mesh passed on by the node, the input mesh when not called
//     Arrays are not bounds checked. Counts can be lowered, to grow a mesh fill a new mesh_geo_create(vertex_count, index_count).
//     Triangles are counter clockwise, a mesh fits the default view at a size of about 1.
//     mesh_geo_* functions return a new mesh and leave their arguments unchanged, meshes are freed when the script ends.
//     Except mesh_geo_append(dst, src, m), which adds src to dst in place (dst counts grow, false when its capacity is
//     full), transformed by float m[12]: the x, y, z axes then the translation. Use it to place many copies of a mesh.
//     mesh_geo_shape: 0 Box, 1 Sphere, 2 Cylinder, 3 Cone, 4 Torus, 5 Plane. mesh_geo_mirror axis: 0 X, 1 Y, 2 Z.
//     mesh_geo_boolean op: MESH_BOOLEAN_UNION, MESH_BOOLEAN_DIFFERENCE, MESH_BOOLEAN_INTERSECT. Rotations are in degrees.
//     Call mesh_geo_normals(g, smooth) after moving vertices. The input passes through when the script has an error.
//     Inputs: script_mesh_add_input(o, sc, "VALUE", "Height") adds a VALUE, VECTOR, RGBA or MESH socket after the mesh input
//     (socket 1, 2, ..), the Add button of the node does the same. The script reads them by name with
//     mesh_geo_input_value("Height") and mesh_geo_input_vector("Offset", i), i is 0 - 2 for x y z, 0 - 3 for r g b a.
//     Value, Math and Vector nodes can be linked into them.
//     script_timeline_get_frame() returns the timeline frame, a script that calls it is rerun when the frame changes.
//     Globals start over on every run. void *mesh_geo_state(int size) returns a zeroed block of size bytes that the
//     node keeps between its runs, to cache work such as simulated frames. A different size gives a new zeroed block,
//     store the inputs it was built from in it to tell when to start over.
//     A "MESH" input takes a second mesh, such as a MESH_OBJECT node (button 0 is the scene mesh index), read with
//     mesh_geo_t *path = mesh_geo_input_mesh("Path"); it is in the space of the edited object, empty when not linked.
//     TEX_IMAGE (Image Texture) node: button 0 is the index of a project texture (Textures tab), outputs Color (0) and
//     Alpha (1). Link one into an RGBA or VALUE input and read its pixels with mesh_geo_image_t *img =
//     mesh_geo_input_image("Mask"); img->width, img->height, mesh_geo_image_sample(img, u, v,
//     channel) samples the nearest pixel at a mesh uv (0 - 1, beyond for 16 and 32 bit layers).
//     The Alpha output gives alpha in all channels. An input without an image gives a 1x1 image of its value.
//     LAYER_MASK (Layer Mask) node: button 0 is the index of a project layer, reads like an image with the red channel
//     in all four, mesh_geo_input_image() of a VALUE input linked to it.
//     Example:
//     mesh_object_t *o = script_get_context()->paint_object;
//     ui_node_t *sc = script_mesh_create_node_at(o, "MESH_SCRIPT", 300.0, 100.0);
//     script_material_set_text(sc, 0, "void main() {\nmesh_geo_t *g = mesh_geo_subdivide(mesh_geo_input(), 2);\nfor (int i = 0; i < g->vertex_count; i++) {\ng->pos[i * 3 + 2] += sin(g->pos[i * 3] * 8.0) * 0.05;\n}\ng = mesh_geo_boolean(mesh_geo_normals(g, true), mesh_geo_shape(1, 0.8, 0.8, 0.8, 32), MESH_BOOLEAN_DIFFERENCE);\nmesh_geo_output(g);\n}");
//     script_mesh_connect(o, script_mesh_get_node(o, "MESH_INPUT"), 0, sc, 0);
//     script_mesh_connect(o, sc, 0, script_mesh_get_node(o, "MESH_OUTPUT"), 0);
//     script_mesh_update(o);
