#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <PR/ultratypes.h>
#include <PR/gbi.h>
#include "platform.h"
#include "config.h"
#include "video.h"

#include "../fast3d/gfx_api.h"
#include "../fast3d/gfx_sdl.h"
#include "../fast3d/gfx_opengl.h"

#ifdef PLATFORM_NSWITCH
#define DEFAULT_VID_WIDTH 1280
#define DEFAULT_VID_HEIGHT 720
#define DEFAULT_VID_FULLSCREEN true
#define DEFAULT_VID_FULLSCREEN_EXCLUSIVE true
#else
#define DEFAULT_VID_WIDTH 640
#define DEFAULT_VID_HEIGHT 480
#define DEFAULT_VID_FULLSCREEN false
#define DEFAULT_VID_FULLSCREEN_EXCLUSIVE false
#endif

static struct GfxWindowManagerAPI *wmAPI;
static struct GfxRenderingAPI *renderingAPI;

static bool initDone = false;

static s32 vidWidth = DEFAULT_VID_WIDTH;
static s32 vidHeight = DEFAULT_VID_HEIGHT;
static s32 vidFramebuffers = true;
static s32 vidFullscreen = DEFAULT_VID_FULLSCREEN;
static s32 vidFullscreenExclusive = DEFAULT_VID_FULLSCREEN_EXCLUSIVE;
static s32 vidMaximize = false;
static s32 vidAllowHiDpi = false;
static s32 vidVsync = 1;
static s32 vidMSAA = 1;
static s32 vidFramerateLimit = 0;

static s32 texFilter = FILTER_LINEAR;
static s32 texFilter2D = true;
static s32 texDetail = true;

static u32 dlcount = 0;
static u32 frames = 0;
static u32 framesPerSec = 0;
static f64 startTime, endTime, fpsTime;

s32 videoInit(void)
{
	wmAPI = &gfx_sdl;
	renderingAPI = &gfx_opengl_api;

	gfx_current_native_viewport.width = 320;
	gfx_current_native_viewport.height = 220;
	gfx_current_native_aspect = 320.f / 220.f;
	gfx_framebuffers_enabled = (bool)vidFramebuffers;
	gfx_detail_textures_enabled = (bool)texDetail;
	gfx_msaa_level = vidMSAA;

	struct GfxInitSettings set = {
		.wapi = wmAPI,
		.rapi = renderingAPI,
		.window_settings = {
			.title = "Perfect Dark",
			.width = vidWidth,
			.height = vidHeight,
			.x = 100,
			.y = 100,
			.fullscreen = vidFullscreen,
			.fullscreen_is_exclusive = vidFullscreenExclusive,
			.maximized = vidMaximize,
			.allow_hidpi = vidAllowHiDpi
		}
	};

	gfx_init(&set);

	if (!wmAPI->set_swap_interval(vidVsync)) {
		vidVsync = 0;
	}

	if (vidVsync == 0 && vidFramerateLimit == 0) {
		// cap FPS if there's no vsync to prevent the game from exploding
		vidFramerateLimit = VIDEO_MAX_FPS;
	}

	wmAPI->set_target_fps(vidFramerateLimit); // disabled because vsync is on
	gfx_set_texture_filter((enum FilteringMode)texFilter);

	initDone = true;
	return 0;
}

void videoStartFrame(void)
{
	if (initDone) {
		startTime = wmAPI->get_time();
		gfx_start_frame();
	}
}

void videoSubmitCommands(Gfx *cmds)
{
	if (initDone) {
		gfx_run(cmds);
		++dlcount;
	}
}

void videoEndFrame(void)
{
	if (!initDone) {
		return;
	}

	gfx_end_frame();

	endTime = wmAPI->get_time();

	++frames;
	++framesPerSec;

	if (endTime >= fpsTime) {
		char tmp[128];
		snprintf(tmp, sizeof(tmp), "fps %3u frt %lf frm %u", framesPerSec, endTime - startTime, frames);
		wmAPI->set_window_title(tmp);
		framesPerSec = 0;
		fpsTime = endTime + 1.0;
	}
}

void videoClearScreen(void)
{
	videoStartFrame();
	// TODO: clear
	videoEndFrame();
}

void *videoGetWindowHandle(void)
{
	if (initDone) {
		return wmAPI->get_window_handle();
	}
	return NULL;
}

void videoUpdateNativeResolution(s32 w, s32 h)
{
	gfx_current_native_viewport.width = w;
	gfx_current_native_viewport.height = h;
	gfx_current_native_aspect = (float)w / (float)h;
}

s32 videoGetNativeWidth(void)
{
	return gfx_current_native_viewport.width;
}

s32 videoGetNativeHeight(void)
{
	return gfx_current_native_viewport.height;
}

s32 videoGetWidth(void)
{
	return gfx_current_dimensions.width;
}

s32 videoGetHeight(void)
{
	return gfx_current_dimensions.height;
}

s32 videoGetFullscreen(void)
{
	return vidFullscreen;
}

s32 videoGetMaximizeWindow(void)
{
	return vidMaximize;
}

f32 videoGetAspect(void)
{
	return gfx_current_dimensions.aspect_ratio;
}

s32 videoGetTextureFilter2D(void)
{
	return texFilter2D;
}

u32 videoGetTextureFilter(void)
{
	return texFilter;
}

s32 videoGetDetailTextures(void)
{
	return texDetail;
}

void videoSetWindowOffset(s32 x, s32 y)
{
	gfx_current_game_window_viewport.x = x;
	gfx_current_game_window_viewport.y = y;
}

void videoSetFullscreen(s32 fs)
{
	if (fs != vidFullscreen) {
		vidFullscreen = !!fs;
		wmAPI->set_fullscreen(vidFullscreen);
	}
}

void videoSetMaximizeWindow(s32 fs)
{
	if (fs != vidMaximize) {
		vidMaximize = !!fs;
		wmAPI->set_maximize(vidMaximize);
	}
}

void videoSetTextureFilter(u32 filter)
{
	if (filter > FILTER_THREE_POINT) filter = FILTER_THREE_POINT;
	if (texFilter == filter) return;
	texFilter = filter;
	gfx_set_texture_filter((enum FilteringMode)filter);
}

void videoSetTextureFilter2D(s32 filter)
{
	texFilter2D = !!filter;
}

void videoSetDetailTextures(s32 detail)
{
	texDetail = !!detail;
	gfx_detail_textures_enabled = (bool)texDetail;
}

s32 videoCreateFramebuffer(u32 w, u32 h, s32 upscale, s32 autoresize)
{
	return gfx_create_framebuffer(w, h, upscale, autoresize);
}

void videoSetFramebuffer(s32 target)
{
	return gfx_set_framebuffer(target, 1.f);
}

void videoResetFramebuffer(void)
{
	return gfx_reset_framebuffer();
}

s32 videoFramebuffersSupported(void)
{
	return gfx_framebuffers_enabled;
}

void videoResizeFramebuffer(s32 target, u32 w, u32 h, s32 upscale, s32 autoresize)
{
	gfx_resize_framebuffer(target, w, h, upscale, autoresize);
}

void videoCopyFramebuffer(s32 dst, s32 src, s32 left, s32 top)
{
	// assume immediate copies always read the front buffer
	gfx_copy_framebuffer(dst, src, left, top, false);
}

void videoResetTextureCache(void)
{
	gfx_texture_cache_clear();
}

void videoFreeCachedTexture(const void *texptr)
{
	gfx_texture_cache_delete(texptr);
}

PD_CONSTRUCTOR static void videoConfigInit(void)
{
	configRegisterInt("Video.DefaultFullscreen", &vidFullscreen, 0, 1);
	configRegisterInt("Video.DefaultMaximize", &vidMaximize, 0, 1);
	configRegisterInt("Video.DefaultWidth", &vidWidth, 0, 32767);
	configRegisterInt("Video.DefaultHeight", &vidHeight, 0, 32767);
	configRegisterInt("Video.ExclusiveFullscreen", &vidFullscreenExclusive, 0, 1);
	configRegisterInt("Video.AllowHiDpi", &vidAllowHiDpi, 0, 1);
	configRegisterInt("Video.VSync", &vidVsync, -1, 10);
	configRegisterInt("Video.FramebufferEffects", &vidFramebuffers, 0, 1);
	configRegisterInt("Video.FramerateLimit", &vidFramerateLimit, 0, VIDEO_MAX_FPS);
	configRegisterInt("Video.MSAA", &vidMSAA, 1, 16);
	configRegisterInt("Video.TextureFilter", &texFilter, 0, 2);
	configRegisterInt("Video.TextureFilter2D", &texFilter2D, 0, 1);
	configRegisterInt("Video.DetailTextures", &texDetail, 0, 1);
}
