#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <PR/ultratypes.h>
#include <PR/gbi.h>
#include "platform.h"
#include "config.h"
#include "system.h"
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
static s32 vidCenter = false;
static s32 vidAllowHiDpi = false;
static s32 vidVsync = 1;
static s32 vidMSAA = 1;
static s32 vidFramerateLimit = 0;
static s32 vidDisplayFPS = 0;
static s32 vidDisplayFPSDivisor = 16;
static displaymode vidModeDefault;
static s32 vidNumModes = 1;
static displaymode *vidModes = &vidModeDefault;
static f64 vidDiffTime;

static s32 texFilter = FILTER_LINEAR;
static s32 texFilter2D = true;
static s32 texDetail = true;

static u32 dlcount = 0;
static u32 frames = 0;
static f64 startTime, endTime;

static s32 videoInitDisplayModes(void);

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
			.centered = vidCenter,
			.allow_hidpi = vidAllowHiDpi
		}
	};

	gfx_init(&set);

	videoInitDisplayModes();
	videoSetVsync(vidVsync);
	videoSetFramerateLimit(vidFramerateLimit);

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

	// Synchronize with their backend counterparts.
	vidFullscreen = videoGetFullscreen();
	vidMaximize = videoGetMaximizeWindow();
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
	static f64 accumTime = 0.0;

	if (!initDone) {
		return;
	}

	gfx_end_frame();

	++frames;

	endTime = wmAPI->get_time();
	accumTime += endTime - startTime;

	if (accumTime >= 1.0 / (f64)vidDisplayFPSDivisor) {
		char tmp[128];
		vidDiffTime = endTime - startTime;
		accumTime = 0.0;
		snprintf(tmp, sizeof(tmp), "fps %3u frt %lf frm %u", (u32)(1.0 / vidDiffTime), vidDiffTime, frames);
		wmAPI->set_window_title(tmp);
	}
}

f64 videoGetDiffTime(void)
{
	return vidDiffTime;
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
	vidFullscreen = wmAPI->get_fullscreen_state();
	return vidFullscreen;
}

s32 videoGetFullscreenMode(void)
{
	vidFullscreenExclusive = wmAPI->get_fullscreen_flag_mode();
	return vidFullscreenExclusive;
}

s32 videoGetMaximizeWindow(void)
{
	vidMaximize = wmAPI->get_maximized_state();
	return vidMaximize;
}

s32 videoGetCenterWindow(void)
{
	return vidCenter;
}

f32 videoGetAspect(void)
{
	return gfx_current_dimensions.aspect_ratio;
}

s32 videoGetDisplayModeIndex(void)
{
	for (s32 i = 1; i < vidNumModes; ++i) {
		if (vidModes[i].width == gfx_current_dimensions.width &&
		    vidModes[i].height == gfx_current_dimensions.height) {
			return i;
		}
	}
	// Current dimensions don't match any known mode, so return index 0, "Custom".
	return 0;
}

s32 videoGetMSAA(void)
{
	vidMSAA = (s32)gfx_msaa_level;
	return vidMSAA;
}

s32 videoGetVsync(void)
{
	vidVsync = wmAPI->get_swap_interval();
	return vidVsync;
}

s32 videoGetFramerateLimit(void)
{
	vidFramerateLimit = wmAPI->get_target_fps();
	return vidFramerateLimit;
}

s32 videoGetDisplayFPS(void)
{
	return vidDisplayFPS;
}

s32 videoGetDisplayFPSDivisor(void)
{
	return vidDisplayFPSDivisor;
}

static s32 videoInitDisplayModes(void)
{
	if (!wmAPI->get_current_display_mode(&vidModeDefault.width, &vidModeDefault.height)) {
		vidModeDefault.width = 640;
		vidModeDefault.height = 480;
		return false;
	}

	const s32 numBaseModes = wmAPI->get_num_display_modes();
	if (!numBaseModes) {
		return false;
	}

	const s32 numCustomModes = 1;
	displaymode *modeList = sysMemZeroAlloc((numBaseModes + numCustomModes) * sizeof(displaymode));
	if (!modeList) {
		return false;
	}

	modeList[0].width = 0;
	modeList[0].height = 0;

	s32 numModes = 1;
	s32 w = -1, h = w, neww = w, newh = w;

	// SDL modes are guaranteed to be sorted high to low
	for (s32 i = 0; i < numBaseModes; ++i) {
		wmAPI->get_display_mode(i, &neww, &newh);

		if (neww != w || newh != h) {
			w = neww;
			h = newh;
			modeList[numModes].width = w;
			modeList[numModes].height = h;
			++numModes;
		}
	}

	modeList = sysMemRealloc(modeList, numModes * sizeof(displaymode));
	if (!modeList) {
		return false;
	}

	vidModes = modeList;
	vidNumModes = numModes;

	return true;
}

s32 videoGetDisplayMode(displaymode *out, const s32 index)
{
	if (index >= 0 && index < vidNumModes) {
		*out = vidModes[index];
		return true;
	}
	return false;
}

s32 videoGetNumDisplayModes(void)
{
	return vidNumModes;
}

void videoSetDisplayMode(const s32 index)
{
	const displaymode dm = vidModes[index];

	if (index == 0) {
		// "Custom" video mode.
		return;
	}

	vidWidth = dm.width;
	vidHeight = dm.height;

	s32 posX = 100;
	s32 posY = 100;
	if (vidCenter) {
		wmAPI->get_centered_positions(vidWidth, vidHeight, &posX, &posY);
	}

	if (vidFullscreen) {
		wmAPI->set_closest_resolution(vidWidth, vidHeight, vidCenter);
	} else {
		if (vidMaximize) {
			videoSetMaximizeWindow(false);
		} else {
			wmAPI->set_dimensions(vidWidth, vidHeight, posX, posY);
		}
	}
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
		wmAPI->set_closest_resolution(vidWidth, vidHeight, vidCenter);
		wmAPI->set_fullscreen(vidFullscreen);
		if (!vidFullscreen && vidMaximize) {
			wmAPI->set_maximize(false);
			wmAPI->set_maximize(true);
		}
	}
}

void videoSetFullscreenMode(s32 mode)
{
	vidFullscreenExclusive = mode;
	wmAPI->set_fullscreen_flag(mode);
	if (vidFullscreen) {
		wmAPI->set_fullscreen(false);
		wmAPI->set_fullscreen(true);
	}
}

void videoSetMaximizeWindow(s32 fs)
{
	if (fs != vidMaximize) {
		vidMaximize = !!fs;
		wmAPI->set_maximize(vidMaximize);
		if (vidCenter && !vidMaximize) {
			s32 posX = 0;
			s32 posY = 0;
			wmAPI->get_centered_positions(vidWidth, vidHeight, &posX, &posY);
			wmAPI->set_dimensions(vidWidth, vidHeight, posX, posY);
		}
	}
}

void videoSetCenterWindow(s32 center)
{
	vidCenter = center;
	if (vidCenter && !vidMaximize) {
		s32 posX = 0;
		s32 posY = 0;
		wmAPI->get_centered_positions(vidWidth, vidHeight, &posX, &posY);
		wmAPI->set_dimensions(vidWidth, vidHeight, posX, posY);
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

void videoSetMSAA(const s32 msaa)
{
	vidMSAA = msaa;
	gfx_msaa_level = (u32)vidMSAA;
}

void videoSetVsync(const s32 vsync)
{
	vidVsync = wmAPI->set_swap_interval(vsync) ? vsync : 0;

	if (vidVsync == 0 && vidFramerateLimit == 0) {
		// cap FPS if there's no vsync to prevent the game from exploding
		videoSetFramerateLimit(VIDEO_MAX_FPS);
	}
}

void videoSetFramerateLimit(const s32 limit)
{
	vidFramerateLimit = (vidVsync == 0 && limit == 0) ? VIDEO_MAX_FPS : limit;
	wmAPI->set_target_fps(vidFramerateLimit);
}

void videoSetDisplayFPS(const s32 displayfps)
{
	vidDisplayFPS = displayfps;
}

void videoSetDisplayFPSDivisor(const s32 divisor)
{
	vidDisplayFPSDivisor = divisor;
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

void videoShutdown(void)
{
	free(vidModes);
}

PD_CONSTRUCTOR static void videoConfigInit(void)
{
	configRegisterInt("Video.DefaultFullscreen", &vidFullscreen, 0, 1);
	configRegisterInt("Video.DefaultMaximize", &vidMaximize, 0, 1);
	configRegisterInt("Video.DefaultWidth", &vidWidth, 0, 32767);
	configRegisterInt("Video.DefaultHeight", &vidHeight, 0, 32767);
	configRegisterInt("Video.ExclusiveFullscreen", &vidFullscreenExclusive, 0, 1);
	configRegisterInt("Video.CenterWindow", &vidCenter, 0, 1);
	configRegisterInt("Video.AllowHiDpi", &vidAllowHiDpi, 0, 1);
	configRegisterInt("Video.VSync", &vidVsync, -1, 10);
	configRegisterInt("Video.FramebufferEffects", &vidFramebuffers, 0, 1);
	configRegisterInt("Video.FramerateLimit", &vidFramerateLimit, 0, VIDEO_MAX_FPS);
	configRegisterInt("Video.DisplayFPS", &vidDisplayFPS, 0, 1);
	configRegisterInt("Video.DisplayFPSDivisor", &vidDisplayFPSDivisor, 1, 32);
	configRegisterInt("Video.MSAA", &vidMSAA, 1, 16);
	configRegisterInt("Video.TextureFilter", &texFilter, 0, 2);
	configRegisterInt("Video.TextureFilter2D", &texFilter2D, 0, 1);
	configRegisterInt("Video.DetailTextures", &texDetail, 0, 1);
}
