#ifndef GFX_WINDOW_MANAGER_API_H
#define GFX_WINDOW_MANAGER_API_H

#include <stdint.h>
#include <stdbool.h>

struct GfxWindowInitSettings {
    const char *title;
    uint32_t width;
    uint32_t height;
    int32_t x;
    int32_t y;
    bool fullscreen;
    bool fullscreen_is_exclusive;
    bool maximized;
    bool centered;
    bool allow_hidpi;
};

struct GfxWindowManagerAPI {
    void (*init)(const struct GfxWindowInitSettings *settings);
    void (*close)(void);
    int (*get_display_mode)(int modenum, int *out_w, int *out_h);
    int (*get_current_display_mode)(int *out_w, int *out_h);
    int (*get_num_display_modes)(void);
    int32_t (*get_fullscreen_state)(void);
    void (*set_fullscreen_changed_callback)(void (*on_fullscreen_changed)(bool is_now_fullscreen));
    void (*set_fullscreen)(bool enable);
    void (*set_fullscreen_exclusive)(bool exc);
    void (*set_fullscreen_flag)(int32_t mode);
    int32_t (*get_fullscreen_flag_mode)(void);
    int32_t (*get_maximized_state)(void);
    void (*set_maximize)(bool enable);
    void (*get_active_window_refresh_rate)(uint32_t* refresh_rate);
    void (*set_cursor_visibility)(bool visible);
    void (*set_closest_resolution)(int32_t width, int32_t height, bool should_center);
    void (*set_dimensions)(uint32_t width, uint32_t height, int32_t posX, int32_t posY);
    void (*get_dimensions)(uint32_t* width, uint32_t* height, int32_t* posX, int32_t* posY);
    void (*get_centered_positions)(int32_t width, int32_t height, int32_t *posX, int32_t *posY);
    void (*handle_events)(void);
    bool (*start_frame)(void);
    void (*swap_buffers_begin)(void);
    void (*swap_buffers_end)(void);
    double (*get_time)(void); // For debug
    int32_t (*get_target_fps)(void);
    void (*set_target_fps)(int fps);
    bool (*can_disable_vsync)(void);
    void *(*get_window_handle)(void);
    void (*set_window_title)(const char *);
    int (*get_swap_interval)(void);
    bool (*set_swap_interval)(int);
};

#endif
