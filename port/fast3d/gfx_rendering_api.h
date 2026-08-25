#ifndef GFX_RENDERING_API_H
#define GFX_RENDERING_API_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

struct ShaderProgram;

struct GfxClipParameters {
    bool z_is_from_0_to_1;
    bool invert_y;
};

enum FilteringMode { FILTER_NONE, FILTER_LINEAR, FILTER_THREE_POINT };
enum MipmapFilteringMode { MIPMAP_DISABLED, MIPMAP_NEAREST, MIPMAP_LINEAR };

struct GfxRenderingAPI {
    const char* (*get_name)(void);
    int (*get_max_texture_size)(void);
    struct GfxClipParameters (*get_clip_parameters)(void);
    void (*unload_shader)(struct ShaderProgram* old_prg);
    void (*load_shader)(struct ShaderProgram* new_prg);
    struct ShaderProgram* (*create_and_load_new_shader)(uint64_t shader_id0, uint32_t shader_id1);
    struct ShaderProgram* (*lookup_shader)(uint64_t shader_id0, uint32_t shader_id1);
    void (*shader_get_info)(struct ShaderProgram* prg, uint8_t* num_inputs, bool used_textures[2]);
    void (*clear_shaders)(void);
    uint32_t (*new_texture)(void);
    void (*select_texture)(int tile, uint32_t texture_id, bool linear_filter);
    void (*upload_texture)(const uint8_t* rgba32_buf, uint32_t width, uint32_t height, bool gen_mipmaps);
    void (*set_sampler_parameters)(int sampler, bool linear_filter, uint32_t cms, uint32_t cmt, bool mipmaps);
    void (*set_depth_mode)(bool depth_test, bool depth_update, bool depth_compare, bool depth_source_prim, uint16_t zmode);
    void (*set_depth_range)(float znear, float zfar);
    void (*set_viewport)(int x, int y, int width, int height);
    void (*set_scissor)(int x, int y, int width, int height);
    void (*set_use_alpha)(bool use_alpha, bool modulate);
    void (*draw_triangles)(float buf_vbo[], size_t buf_vbo_len, size_t buf_vbo_num_tris);
    void (*init)(void);
    void (*on_resize)(void);
    void (*start_frame)(void);
    void (*end_frame)(void);
    void (*finish_render)(void);
    int (*create_framebuffer)();
    void (*update_framebuffer_parameters)(int fb_id, uint32_t width, uint32_t height, uint32_t msaa_level,
                                          bool opengl_invert_y, bool render_target, bool has_depth_buffer,
                                          bool can_extract_depth);
    bool (*start_draw_to_framebuffer)(int fb_id, float noise_scale);
    void (*copy_framebuffer)(int fb_dst, int fb_src, int left, int top, bool flip_y, bool use_back);
    void (*clear_framebuffer)(bool clear_color, bool clear_depth);
    void (*resolve_msaa_color_buffer)(int fb_id_target, int fb_id_source);
    void* (*get_framebuffer_texture_id)(int fb_id);
    void (*select_texture_fb)(int fb_id);
    void (*delete_texture)(uint32_t texID);
    void (*set_texture_filter)(enum FilteringMode mode);
    enum FilteringMode (*get_texture_filter)(void);
    void (*set_mipmap_filter)(enum MipmapFilteringMode mode);
	void (*set_anisotropy_level)(int);
	int (*get_max_anisotropy_level)(void);
};

#endif
