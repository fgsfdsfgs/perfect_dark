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
    bool allow_hidpi;
};

struct GfxWindowManagerAPI {
    void (*init)(const struct GfxWindowInitSettings *settings);
    void (*close)(void);
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
    void (*set_closest_resolution)(int32_t width, int32_t height);
    void (*set_dimensions)(uint32_t width, uint32_t height, int32_t posX, int32_t posY);
    void (*get_dimensions)(uint32_t* width, uint32_t* height, int32_t* posX, int32_t* posY);
    void (*handle_events)(void);
    bool (*start_frame)(void);
    void (*swap_buffers_begin)(void);
    void (*swap_buffers_end)(void);
    double (*get_time)(void); // For debug
    void (*set_target_fps)(int fps);
    bool (*can_disable_vsync)(void);
    void *(*get_window_handle)(void);
    void (*set_window_title)(const char *);
    bool (*set_swap_interval)(int);
    struct GfxModes *(*alloc_display_modes)(struct GfxModes *, int *);
};

#endif
