#include <SDL.h>
#include <PR/ultratypes.h>
#include <PR/os_thread.h>
#include <PR/os_cont.h>
#include "input.h"
#include "video.h"
#include "config.h"

#define MAX_BINDS 4
#define TRIG_THRESHOLD (30 * 256)
#define DEFAULT_DEADZONE 4096
#define DEFAULT_DEADZONE_RY 6144

#define WHEEL_UP_MASK SDL_BUTTON(VK_MOUSE_WHEEL_UP - VK_MOUSE_BEGIN + 1)
#define WHEEL_DN_MASK SDL_BUTTON(VK_MOUSE_WHEEL_DN - VK_MOUSE_BEGIN + 1)

static SDL_GameController *pads[INPUT_MAX_CONTROLLERS];
static u32 binds[MAXCONTROLLERS][CK_TOTAL_COUNT][MAX_BINDS]; // [i][CK_][b] = [VK_]

static s32 connectedMask = 0;

static s32 mouseEnabled = 1;
static s32 mouseLocked = 0;
static s32 mouseX, mouseY;
static s32 mouseDX, mouseDY;
static u32 mouseButtons;
static s32 mouseWheel = 0;

static f32 mouseSensX = 1.5f;
static f32 mouseSensY = 1.5f;

static f32 rumbleScale = 0.333f;

// NOTE: by default this gets inverted for 1.2
static u32 axisMap[2][2] = {
	{ SDL_CONTROLLER_AXIS_LEFTX,  SDL_CONTROLLER_AXIS_LEFTY  },
	{ SDL_CONTROLLER_AXIS_RIGHTX, SDL_CONTROLLER_AXIS_RIGHTY },
};

static f32 stickSens[42] = {
	// index == SDL_CONTROLLER_AXIS_*
	1.f, 1.f, 1.f, 1.f
};

static s32 deadzone[4] = {
	// index == SDL_CONTROLLER_AXIS_*
	DEFAULT_DEADZONE, DEFAULT_DEADZONE,
	DEFAULT_DEADZONE, DEFAULT_DEADZONE_RY,
};

void inputSetDefaultKeyBinds(void)
{
	// TODO: make VK constants for all these
	static const u32 kbbinds[][3] = {
		{ CK_A,             SDL_SCANCODE_Q,      0                   },
		{ CK_B,             SDL_SCANCODE_E,      0                   },
		{ CK_X,             SDL_SCANCODE_R,      0                   },
		{ CK_Y,             SDL_SCANCODE_R,      0                   },
		{ CK_RTRIG,         VK_MOUSE_RIGHT,      SDL_SCANCODE_Z      },
		{ CK_LTRIG,         SDL_SCANCODE_F,      SDL_SCANCODE_X      },
		{ CK_ZTRIG,         VK_MOUSE_LEFT,       SDL_SCANCODE_SPACE  },
		{ CK_START,         SDL_SCANCODE_RETURN, SDL_SCANCODE_TAB    },
		{ CK_DPAD_D,        SDL_SCANCODE_G,      VK_MOUSE_MIDDLE     },
		{ CK_DPAD_U,        0,                   0                   },
		{ CK_DPAD_R,        VK_MOUSE_WHEEL_UP,   0                   },
		{ CK_DPAD_L,        VK_MOUSE_WHEEL_DN,   0                   },
		{ CK_C_D,           SDL_SCANCODE_S,      0                   },
		{ CK_C_U,           SDL_SCANCODE_W,      0                   },
		{ CK_C_R,           SDL_SCANCODE_D,      0                   },
		{ CK_C_L,           SDL_SCANCODE_A,      0                   },
		{ CK_STICK_XNEG,    SDL_SCANCODE_LEFT,   0                   },
		{ CK_STICK_XPOS,    SDL_SCANCODE_RIGHT,  0                   },
		{ CK_STICK_YNEG,    SDL_SCANCODE_DOWN,   0                   },
		{ CK_STICK_YPOS,    SDL_SCANCODE_UP,     0                   },
	};

	static const u32 joybinds[][2] = {
		// { CK_A,      SDL_CONTROLLER_BUTTON_A             },
		{ CK_B,      SDL_CONTROLLER_BUTTON_A             },
		{ CK_X,      SDL_CONTROLLER_BUTTON_X             },
		{ CK_Y,      SDL_CONTROLLER_BUTTON_BACK        },
		{ CK_DPAD_L, SDL_CONTROLLER_BUTTON_Y,            },
		{ CK_DPAD_R, SDL_CONTROLLER_BUTTON_B,            },
		{ CK_DPAD_D, SDL_CONTROLLER_BUTTON_LEFTSHOULDER  },
		{ CK_LTRIG,  SDL_CONTROLLER_BUTTON_RIGHTSHOULDER },
		{ CK_RTRIG,  VK_JOY1_LTRIG - VK_JOY1_BEGIN       },
		{ CK_ZTRIG,  VK_JOY1_RTRIG - VK_JOY1_BEGIN       },
		{ CK_START,  SDL_CONTROLLER_BUTTON_START         },
		{ CK_C_D,    SDL_CONTROLLER_BUTTON_DPAD_DOWN     },
		{ CK_C_U,    SDL_CONTROLLER_BUTTON_DPAD_UP       },
		{ CK_C_R,    SDL_CONTROLLER_BUTTON_DPAD_RIGHT    },
		{ CK_C_L,    SDL_CONTROLLER_BUTTON_DPAD_LEFT     },
	};

	for (u32 i = 0; i < sizeof(kbbinds) / sizeof(kbbinds[0]); ++i) {
		for (s32 j = 1; j < 3; ++j) {
			if (kbbinds[i][j]) {
				inputKeyBind(0, kbbinds[i][0], j - 1, kbbinds[i][j]);
			}
		}
	}

	for (u32 i = 0; i < sizeof(joybinds) / sizeof(joybinds[0]); ++i) {
		for (s32 j = 0; j < INPUT_MAX_CONTROLLERS; ++j) {
			inputKeyBind(j, joybinds[i][0], -1, VK_JOY_BEGIN + j * INPUT_MAX_CONTROLLER_BUTTONS + joybinds[i][1]);
		}
	}
}

static int inputEventFilter(void *data, SDL_Event *event)
{
	switch (event->type) {
		case SDL_CONTROLLERDEVICEADDED:
			for (s32 i = 0; i < INPUT_MAX_CONTROLLERS; ++i) {
				if (!pads[i]) {
					pads[i] = SDL_GameControllerOpen(event->cdevice.which);
					if (pads[i]) {
						connectedMask |= (1 << i);
					}
					break;
				}
			}
			break;

		case SDL_CONTROLLERDEVICEREMOVED: {
			SDL_GameController *ctrl = SDL_GameControllerFromInstanceID(event->cdevice.which);
			if (ctrl) {
				for (s32 i = 0; i < INPUT_MAX_CONTROLLERS; ++i) {
					if (pads[i] == ctrl) {
						SDL_GameControllerClose(pads[i]);
						pads[i] = NULL;
						if (i) {
							connectedMask &= ~(1 << i);
						}
						break;
					}
				}
			}
			break;
		}

		case SDL_MOUSEWHEEL:
			mouseWheel = event->wheel.y;
			break;

		default:
			break;
	}

	return 0;
}

s32 inputInit(void)
{
	if (!SDL_WasInit(SDL_INIT_GAMECONTROLLER)) {
		SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
	}

	const s32 numJoys = SDL_NumJoysticks();

	s32 ret = 1; // always report first controller as connected

	// if this is set to 1, keyboard will count as a separate controller on its own,
	// so the first connected gamepad will go to player 2 instead of player 1
	const s32 cstart = configGetInt("Input.FirstGamepadNum", 0);

	for (s32 jidx = 0, cidx = cstart; jidx < numJoys && cidx < INPUT_MAX_CONTROLLERS; ++jidx) {
		if (SDL_IsGameController(jidx)) {
			pads[cidx] = SDL_GameControllerOpen(jidx);
			if (pads[cidx]) {
				SDL_GameControllerSetPlayerIndex(pads[cidx], cidx);
				ret |= 1 << cidx;
				++cidx;
			}
		}
	}

	// since the main event loop is elsewhere, we can receive some events we need using a watcher
	SDL_AddEventWatch(inputEventFilter, NULL);

	inputSetDefaultKeyBinds();

	mouseEnabled = configGetInt("Input.MouseEnabled", 1);
	mouseSensX = configGetFloat("Input.MouseSpeedX", 1.5f);
	mouseSensY = configGetFloat("Input.MouseSpeedY", 1.5f);

	rumbleScale = configGetFloat("Input.RumbleScale", 0.333f);

	deadzone[0] = configGetInt("Input.LStickDeadzoneX", DEFAULT_DEADZONE);
	deadzone[1] = configGetInt("Input.LStickDeadzoneY", DEFAULT_DEADZONE);
	deadzone[2] = configGetInt("Input.RStickDeadzoneX", DEFAULT_DEADZONE);
	deadzone[3] = configGetInt("Input.RStickDeadzoneY", DEFAULT_DEADZONE_RY);

	stickSens[0] = configGetFloat("Input.LStickScaleX", 1.f);
	stickSens[1] = configGetFloat("Input.LStickScaleY", 1.f);
	stickSens[2] = configGetFloat("Input.RStickScaleX", 1.f);
	stickSens[3] = configGetFloat("Input.RStickScaleY", 1.f);

	if (configGetInt("Input.SwapSticks", 1)) {
		// invert axis map
		axisMap[0][0] = SDL_CONTROLLER_AXIS_RIGHTX;
		axisMap[0][1] = SDL_CONTROLLER_AXIS_RIGHTY;
		axisMap[1][0] = SDL_CONTROLLER_AXIS_LEFTX;
		axisMap[1][1] = SDL_CONTROLLER_AXIS_LEFTY;
	}

	const s32 overrideMask = (1 << configGetInt("Input.FakeGamepads", 0)) - 1;
	if (overrideMask) {
		ret = overrideMask;
	}

	connectedMask = ret;

	return ret;
}

static inline s32 inputBindPressed(const s32 idx, const u32 ck)
{
	for (s32 i = 0; i < MAX_BINDS; ++i) {
		if (binds[idx][ck][i]) {
			if (inputKeyPressed(binds[idx][ck][i])) {
				return 1;
			}
		}
	}
	return 0;
}

static inline s32 inputAxisScale(s32 x, const s32 deadzone, const f32 scale)
{
	if (abs(x) < deadzone) {
		return 0;
	} else {
		// rescale to fit the non-deadzone range
		if (x < 0) {
			x += deadzone;
		} else {
			x -= deadzone;
		}
		x = x * 32768 / (32768 - deadzone);
		// scale with sensitivity
		x *= scale;
		return (x > 32767) ? 32767 : ((x < -32768) ? -32768 : x);
	}
}

s32 inputReadController(s32 idx, OSContPad *npad)
{
	if (idx < 0 || idx >= INPUT_MAX_CONTROLLERS  || !npad) {
		return -1;
	}

	npad->button = 0;

	for (u32 i = 0; i < CONT_NUM_BUTTONS; ++i) {
		if (inputBindPressed(idx, i)) {
			npad->button |= 1U << i;
		}
	}

	const s32 xdiff = (inputBindPressed(idx, CK_STICK_XPOS) - inputBindPressed(idx, CK_STICK_XNEG));
	const s32 ydiff = (inputBindPressed(idx, CK_STICK_YPOS) - inputBindPressed(idx, CK_STICK_YNEG));
	npad->stick_x = xdiff < 0 ? -0x80 : (xdiff > 0 ? 0x7F : 0);
	npad->stick_y = ydiff < 0 ? -0x80 : (ydiff > 0 ? 0x7F : 0);

	if (!pads[idx]) {
		return 0;
	}

	s32 leftX = SDL_GameControllerGetAxis(pads[idx], axisMap[0][0]);
	s32 leftY = SDL_GameControllerGetAxis(pads[idx], axisMap[0][1]);
	s32 rightX = SDL_GameControllerGetAxis(pads[idx], axisMap[1][0]);
	s32 rightY = SDL_GameControllerGetAxis(pads[idx], axisMap[1][1]);

	leftX = inputAxisScale(leftX, deadzone[axisMap[0][0]], stickSens[axisMap[0][0]]);
	leftY = inputAxisScale(leftY, deadzone[axisMap[0][1]], stickSens[axisMap[0][1]]);
	rightX = inputAxisScale(rightX, deadzone[axisMap[1][0]], stickSens[axisMap[1][0]]);
	rightY = inputAxisScale(rightY, deadzone[axisMap[1][1]], stickSens[axisMap[1][1]]);

	if (rightX < -0x4000) npad->button |= L_CBUTTONS;
	if (rightX > +0x4000) npad->button |= R_CBUTTONS;
	if (rightY < -0x4000) npad->button |= U_CBUTTONS;
	if (rightY > +0x4000) npad->button |= D_CBUTTONS;

	if (!npad->stick_x && leftX) {
		npad->stick_x = leftX / 0x100;
	}

	const s32 stickY = -leftY / 0x100;
	if (!npad->stick_y && stickY) {
		npad->stick_y = (stickY == 128) ? 127 : stickY;
	}

	return 0;
}

static inline void inputUpdateMouse(void)
{
	s32 mx, my;
	mouseButtons = SDL_GetMouseState(&mx, &my);

	if (mouseWheel > 0) {
		mouseButtons |= WHEEL_UP_MASK;
	} else if (mouseWheel < 0) {
		mouseButtons |= WHEEL_DN_MASK;
	}

	mouseWheel = 0;

	if (mouseLocked) {
		SDL_GetRelativeMouseState(&mouseDX, &mouseDY);
	} else {
		mouseDX = mx - mouseX;
		mouseDY = my - mouseY;
	}

	mouseX = mx;
	mouseY = my;

	if (!mouseLocked && (mouseButtons & SDL_BUTTON_LMASK)) {
		inputLockMouse(1);
	} else if (mouseLocked && inputKeyPressed(VK_ESCAPE)) {
		inputLockMouse(0);
	}
}

void inputUpdate(void)
{
	SDL_GameControllerUpdate();

	if (mouseEnabled) {
		inputUpdateMouse();
	}
}

s32 inputControllerConnected(s32 idx)
{
	if (idx < 0 || idx >= INPUT_MAX_CONTROLLERS) {
		return 0;
	}
	return pads[idx] || (connectedMask & (1 << idx));
}

s32 inputRumbleSupported(s32 idx)
{
	if (idx < 0 || idx >= INPUT_MAX_CONTROLLERS) {
		return 0;
	}
	return SDL_GameControllerHasRumble(pads[idx]);
}

void inputRumble(s32 idx, f32 strength, f32 time)
{
	if (idx < 0 || idx >= INPUT_MAX_CONTROLLERS || !pads[idx]) {
		return;
	}
	if (SDL_GameControllerHasRumble(pads[idx])) {
		if (strength <= 0.f) {
			strength = 0.f;
			time = 0.f;
		} else {
			strength *= 65535.f;
			time *= 1000.f;
		}
		SDL_GameControllerRumble(pads[idx], (u16)strength, (u16)strength, (u32)time);
	}
}

s32 inputControllerMask(void)
{
	return connectedMask;
}

void inputKeyBind(s32 idx, u32 ck, s32 bind, u32 vk)
{
	if (idx < 0 || idx >= INPUT_MAX_CONTROLLERS || bind >= MAX_BINDS || ck >= CK_TOTAL_COUNT) {
		return;
	}

	if (bind < 0) {
		for (s32 i = 0; i < MAX_BINDS; ++i) {
			if (binds[idx][ck][i] == 0) {
				bind = i;
				break;
			}
		}
		if (bind < 0) {
			bind = MAX_BINDS - 1; // just overwrite last
		}
	}

	binds[idx][ck][bind] = vk;
}

s32 inputKeyPressed(u32 vk)
{
	if (vk >= VK_KEYBOARD_BEGIN && vk < VK_MOUSE_BEGIN) {
		const u8 *state = SDL_GetKeyboardState(NULL);
		return state[vk - VK_KEYBOARD_BEGIN];
	} else if (vk >= VK_MOUSE_BEGIN && vk < VK_JOY_BEGIN) {
		return mouseButtons & SDL_BUTTON(vk - VK_MOUSE_BEGIN + 1);
	} else if (vk >= VK_JOY_BEGIN && vk < VK_TOTAL_COUNT) {
		vk -= VK_JOY_BEGIN;
		const s32 idx = vk / INPUT_MAX_CONTROLLER_BUTTONS;
		if (idx < 0 || idx >= INPUT_MAX_CONTROLLERS || !pads[idx]) {
			return 0;
		}
		vk = vk % INPUT_MAX_CONTROLLER_BUTTONS;
		// triggers
		if (vk == 30 || vk == 31) {
			const s32 trig = SDL_CONTROLLER_AXIS_TRIGGERLEFT + vk - 30;
			return SDL_GameControllerGetAxis(pads[idx], trig) > TRIG_THRESHOLD;
		}
		return SDL_GameControllerGetButton(pads[idx], vk);
	}
	return 0;
}

static inline u16 inputContToContKey(const u16 cont)
{
	if (cont == 0) {
		return 0;
	}
	// just a log2 to convert CONT_* to their indices
	return 32 - __builtin_clz(cont - 1);
}

s32 inputButtonPressed(s32 idx, u16 contbtn)
{
	if (idx < 0 || idx >= INPUT_MAX_CONTROLLERS) {
		return 0;
	}

	return inputBindPressed(idx, inputContToContKey(contbtn));
}

void inputLockMouse(s32 lock)
{
	mouseLocked = !!lock;
	SDL_SetRelativeMouseMode(mouseLocked);
}

s32 inputMouseIsLocked(void)
{
	return mouseLocked;
}

void inputMouseGetPosition(s32 *x, s32 *y)
{
	if (x) *x = mouseX * videoGetNativeWidth() / videoGetWidth();
	if (y) *y = mouseY * videoGetNativeHeight() / videoGetHeight();
}

void inputMouseGetRawDelta(s32 *dx, s32 *dy)
{
	if (dx) *dx = mouseDX;
	if (dy) *dy = mouseDY;
}

void inputMouseGetScaledDelta(f32 *dx, f32 *dy)
{
	f32 mdx, mdy;
	if (mouseLocked) {
		mdx = mouseSensX * (f32)mouseDX / 100.0f;
		mdy = mouseSensY * (f32)mouseDY / 100.0f;
	} else {
		mdx = 0.f;
		mdy = 0.f;
	}
	if (dx) *dx = mdx;
	if (dy) *dy = mdy;
}
