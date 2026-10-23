// SDL.h -- the part of SDL 2 OpenTyrian uses, and no more, over libjs's
// display and sound card (c4sdl.c). The game draws into 8-bit surfaces of
// its own and shows them through video-c4.c; its music and sound effects
// go to the card through loudness-c4.c; this is the rest: surfaces, the
// event queue, the clock, the log, and a joystick that is never there.
// Built by c4lc for C4IX, C4DOS and the bare machine, and by gcc for the
// host (with tyr-host.c), the reference make test-tyrian holds them to.

#ifndef C4TYR_SDL_H
#define C4TYR_SDL_H 1

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t  Uint8;
typedef int8_t   Sint8;
typedef uint16_t Uint16;
typedef int16_t  Sint16;
typedef uint32_t Uint32;
typedef int32_t  Sint32;
typedef uint64_t Uint64;
typedef int64_t  Sint64;
typedef enum { SDL_FALSE = 0, SDL_TRUE = 1 } SDL_bool;

#define SDL_VERSION_ATLEAST(a, b, c) 1
// little-endian, as both the VM and x86 are (file.h tests it)
#define SDL_LIL_ENDIAN 1234
#define SDL_BIG_ENDIAN 4321
#define SDL_BYTEORDER SDL_LIL_ENDIAN
#define SDL_PRINTF_FORMAT_STRING
#define SDL_PRINTF_VARARG_FUNC(n)
#define SDL_SwapLE16(x) (x)
#define SDL_SwapLE32(x) (x)
#define SDL_SwapBE16(x) ((Uint16) ((((x) & 0xff) << 8) | (((x) >> 8) & 0xff)))
#define SDL_SwapBE32(x) ((((Uint32) (x) & 0xff) << 24) | (((Uint32) (x) & 0xff00) << 8) | \
                         (((Uint32) (x) >> 8) & 0xff00) | (((Uint32) (x) >> 24) & 0xff))
#define SDL_free free
size_t SDL_strlcpy(char *dst, const char *src, size_t size);

// ---- init ----
enum {
	SDL_INIT_TIMER = 0x1, SDL_INIT_AUDIO = 0x10, SDL_INIT_VIDEO = 0x20,
	SDL_INIT_JOYSTICK = 0x200, SDL_INIT_EVENTS = 0x4000
};
int  SDL_Init(Uint32 flags);
int  SDL_InitSubSystem(Uint32 flags);
void SDL_QuitSubSystem(Uint32 flags);
void SDL_Quit(void);
const char *SDL_GetError(void);
#define SDL_HINT_MOUSE_RELATIVE_SYSTEM_SCALE "SDL_MOUSE_RELATIVE_SYSTEM_SCALE"
SDL_bool SDL_SetHint(const char *name, const char *value);
char *SDL_GetBasePath(void);
enum { SDL_MESSAGEBOX_ERROR = 0x10 };
int SDL_ShowSimpleMessageBox(Uint32 flags, const char *title, const char *message, void *window);

// ---- time ----
Uint32 SDL_GetTicks(void);
void   SDL_Delay(Uint32 ms);

// ---- surfaces: 8 bits a pixel, which is all the game makes ----
typedef struct { Uint8 r, g, b, a; } SDL_Color;
typedef struct { int x, y, w, h; } SDL_Rect;
typedef struct { Uint8 BitsPerPixel; Uint8 BytesPerPixel; } SDL_PixelFormat;
typedef struct {
	Uint32 flags;
	SDL_PixelFormat *format;
	int w, h;
	int pitch;
	void *pixels;
} SDL_Surface;
// never made here, but complete: c4lc lays out what a compared pointer
// points to, and an incomplete struct has no layout
typedef struct SDL_Window { int unused; } SDL_Window;
typedef struct SDL_Texture { int unused; } SDL_Texture;
#define SDL_MUSTLOCK(s) 0
SDL_Surface *SDL_CreateRGBSurface(Uint32 flags, int w, int h, int depth, Uint32 rm, Uint32 gm, Uint32 bm, Uint32 am);
void SDL_FreeSurface(SDL_Surface *s);
int  SDL_FillRect(SDL_Surface *dst, const SDL_Rect *rect, Uint32 color);
int  SDL_BlitSurface(SDL_Surface *src, const SDL_Rect *srect, SDL_Surface *dst, SDL_Rect *drect);
// a colour as the display takes it, 0x00RRGGBB (GUI_BLIT8's palette)
Uint32 SDL_MapRGB(const SDL_PixelFormat *format, Uint8 r, Uint8 g, Uint8 b);
int  SDL_GetNumVideoDisplays(void);
int  SDL_ShowCursor(int toggle);
int  SDL_SetRelativeMouseMode(SDL_bool on);
enum { SDL_QUERY = -1, SDL_IGNORE = 0, SDL_DISABLE = 0, SDL_ENABLE = 1 };

// ---- scancodes: SDL 2's numbers, the USB HID usage codes ----
typedef int SDL_Scancode;
enum {
	SDL_SCANCODE_UNKNOWN = 0,
	SDL_SCANCODE_A = 4, SDL_SCANCODE_B, SDL_SCANCODE_C, SDL_SCANCODE_D, SDL_SCANCODE_E,
	SDL_SCANCODE_F, SDL_SCANCODE_G, SDL_SCANCODE_H, SDL_SCANCODE_I, SDL_SCANCODE_J,
	SDL_SCANCODE_K, SDL_SCANCODE_L, SDL_SCANCODE_M, SDL_SCANCODE_N, SDL_SCANCODE_O,
	SDL_SCANCODE_P, SDL_SCANCODE_Q, SDL_SCANCODE_R, SDL_SCANCODE_S, SDL_SCANCODE_T,
	SDL_SCANCODE_U, SDL_SCANCODE_V, SDL_SCANCODE_W, SDL_SCANCODE_X, SDL_SCANCODE_Y,
	SDL_SCANCODE_Z,
	SDL_SCANCODE_1 = 30, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4, SDL_SCANCODE_5,
	SDL_SCANCODE_6, SDL_SCANCODE_7, SDL_SCANCODE_8, SDL_SCANCODE_9, SDL_SCANCODE_0,
	SDL_SCANCODE_RETURN = 40, SDL_SCANCODE_ESCAPE = 41, SDL_SCANCODE_BACKSPACE = 42,
	SDL_SCANCODE_TAB = 43, SDL_SCANCODE_SPACE = 44, SDL_SCANCODE_MINUS = 45,
	SDL_SCANCODE_EQUALS = 46, SDL_SCANCODE_LEFTBRACKET = 47, SDL_SCANCODE_RIGHTBRACKET = 48,
	SDL_SCANCODE_BACKSLASH = 49, SDL_SCANCODE_SEMICOLON = 51, SDL_SCANCODE_APOSTROPHE = 52,
	SDL_SCANCODE_GRAVE = 53, SDL_SCANCODE_COMMA = 54, SDL_SCANCODE_PERIOD = 55,
	SDL_SCANCODE_SLASH = 56, SDL_SCANCODE_CAPSLOCK = 57,
	SDL_SCANCODE_F1 = 58, SDL_SCANCODE_F2, SDL_SCANCODE_F3, SDL_SCANCODE_F4, SDL_SCANCODE_F5,
	SDL_SCANCODE_F6, SDL_SCANCODE_F7, SDL_SCANCODE_F8, SDL_SCANCODE_F9, SDL_SCANCODE_F10,
	SDL_SCANCODE_F11, SDL_SCANCODE_F12,
	SDL_SCANCODE_PRINTSCREEN = 70, SDL_SCANCODE_SCROLLLOCK = 71, SDL_SCANCODE_PAUSE = 72,
	SDL_SCANCODE_INSERT = 73, SDL_SCANCODE_HOME = 74, SDL_SCANCODE_PAGEUP = 75,
	SDL_SCANCODE_DELETE = 76, SDL_SCANCODE_END = 77, SDL_SCANCODE_PAGEDOWN = 78,
	SDL_SCANCODE_RIGHT = 79, SDL_SCANCODE_LEFT = 80, SDL_SCANCODE_DOWN = 81, SDL_SCANCODE_UP = 82,
	SDL_SCANCODE_NUMLOCKCLEAR = 83, SDL_SCANCODE_KP_ENTER = 88,
	SDL_SCANCODE_KP_1 = 89, SDL_SCANCODE_KP_2, SDL_SCANCODE_KP_3, SDL_SCANCODE_KP_4,
	SDL_SCANCODE_KP_5, SDL_SCANCODE_KP_6, SDL_SCANCODE_KP_7, SDL_SCANCODE_KP_8,
	SDL_SCANCODE_KP_9, SDL_SCANCODE_KP_0,
	SDL_SCANCODE_LCTRL = 224, SDL_SCANCODE_LSHIFT = 225, SDL_SCANCODE_LALT = 226,
	SDL_SCANCODE_LGUI = 227, SDL_SCANCODE_RCTRL = 228, SDL_SCANCODE_RSHIFT = 229,
	SDL_SCANCODE_RALT = 230,
	SDL_NUM_SCANCODES = 512
};
const char *SDL_GetScancodeName(SDL_Scancode sc);
SDL_Scancode SDL_GetScancodeFromName(const char *name);

// key codes: the character a key types, as SDL's are for printable keys
typedef Sint32 SDL_Keycode;
enum {
	SDLK_UNKNOWN = 0, SDLK_RETURN = '\r', SDLK_ESCAPE = 27, SDLK_BACKSPACE = 8,
	SDLK_TAB = '\t', SDLK_SPACE = ' ', SDLK_RIGHTBRACKET = ']',
	SDLK_d = 'd', SDLK_g = 'g', SDLK_l = 'l', SDLK_o = 'o', SDLK_r = 'r', SDLK_s = 's'
};
typedef enum {
	KMOD_NONE = 0, KMOD_LSHIFT = 1, KMOD_RSHIFT = 2, KMOD_LCTRL = 0x40, KMOD_RCTRL = 0x80,
	KMOD_LALT = 0x100, KMOD_RALT = 0x200,
	KMOD_LGUI = 0x400, KMOD_RGUI = 0x800,
	KMOD_SHIFT = 3, KMOD_CTRL = 0xc0, KMOD_ALT = 0x300, KMOD_GUI = 0xc00
} SDL_Keymod;

// ---- events ----
enum {
	SDL_QUIT = 0x100, SDL_WINDOWEVENT = 0x200, SDL_KEYDOWN = 0x300, SDL_KEYUP = 0x301,
	SDL_TEXTINPUT = 0x303, SDL_MOUSEMOTION = 0x400, SDL_MOUSEBUTTONDOWN = 0x401,
	SDL_MOUSEBUTTONUP = 0x402, SDL_JOYAXISMOTION = 0x600, SDL_USEREVENT = 0x8000
};
enum {
	SDL_WINDOWEVENT_RESIZED = 5, SDL_WINDOWEVENT_FOCUS_GAINED = 12, SDL_WINDOWEVENT_FOCUS_LOST = 13
};
enum { SDL_RELEASED = 0, SDL_PRESSED = 1 };
enum { SDL_BUTTON_LEFT = 1, SDL_BUTTON_MIDDLE = 2, SDL_BUTTON_RIGHT = 3 };
#define SDL_BUTTON(x) (1 << ((x) - 1))
#define SDL_BUTTON_LMASK SDL_BUTTON(SDL_BUTTON_LEFT)
#define SDL_BUTTON_MMASK SDL_BUTTON(SDL_BUTTON_MIDDLE)
#define SDL_BUTTON_RMASK SDL_BUTTON(SDL_BUTTON_RIGHT)
// how often keyboard.c polls, in ms
#define SDL_POLL_INTERVAL 10

typedef struct { SDL_Scancode scancode; SDL_Keycode sym; Uint16 mod; } SDL_Keysym;
typedef struct { Uint32 type; Uint32 timestamp; Uint8 state; Uint8 repeat; SDL_Keysym keysym; } SDL_KeyboardEvent;
typedef struct { Uint32 type; Uint32 timestamp; char text[32]; } SDL_TextInputEvent;
typedef struct { Uint32 type; Uint32 timestamp; Uint32 state; Sint32 x, y, xrel, yrel; } SDL_MouseMotionEvent;
typedef struct { Uint32 type; Uint32 timestamp; Uint8 button; Uint8 state; Uint8 clicks; Sint32 x, y; } SDL_MouseButtonEvent;
typedef struct { Uint32 type; Uint32 timestamp; Uint8 event; Sint32 data1, data2; } SDL_WindowEvent;
typedef struct { Uint32 type; Uint32 timestamp; Sint32 which; Uint8 axis; Sint16 value; } SDL_JoyAxisEvent;
typedef union {
	Uint32 type;
	SDL_KeyboardEvent key;
	SDL_TextInputEvent text;
	SDL_MouseMotionEvent motion;
	SDL_MouseButtonEvent button;
	SDL_WindowEvent window;
	SDL_JoyAxisEvent jaxis;
	Uint8 padding[56];
} SDL_Event;
int  SDL_PollEvent(SDL_Event *ev);
int  SDL_PushEvent(SDL_Event *ev);
void SDL_StartTextInput(void);
void SDL_StopTextInput(void);

// ---- joysticks: there are none ----
typedef struct SDL_Joystick { int unused; } SDL_Joystick;
enum { SDL_HAT_CENTERED = 0, SDL_HAT_UP = 1, SDL_HAT_RIGHT = 2, SDL_HAT_DOWN = 4, SDL_HAT_LEFT = 8 };
int  SDL_NumJoysticks(void);
SDL_Joystick *SDL_JoystickOpen(int i);
void SDL_JoystickClose(SDL_Joystick *j);
const char *SDL_JoystickName(SDL_Joystick *j);
int  SDL_JoystickNumAxes(SDL_Joystick *j);
int  SDL_JoystickNumButtons(SDL_Joystick *j);
int  SDL_JoystickNumHats(SDL_Joystick *j);
Sint16 SDL_JoystickGetAxis(SDL_Joystick *j, int axis);
Uint8 SDL_JoystickGetButton(SDL_Joystick *j, int button);
Uint8 SDL_JoystickGetHat(SDL_Joystick *j, int hat);
void SDL_JoystickUpdate(void);
int  SDL_JoystickEventState(int state);

// ---- the log ----
enum { SDL_LOG_CATEGORY_APPLICATION = 0 };
typedef enum {
	SDL_LOG_PRIORITY_VERBOSE = 1, SDL_LOG_PRIORITY_DEBUG, SDL_LOG_PRIORITY_INFO,
	SDL_LOG_PRIORITY_WARN, SDL_LOG_PRIORITY_ERROR, SDL_LOG_PRIORITY_CRITICAL
} SDL_LogPriority;
typedef void (*SDL_LogOutputFunction)(void *userdata, int category, SDL_LogPriority priority, const char *message);
void SDL_LogGetOutputFunction(SDL_LogOutputFunction *callback, void **userdata);
void SDL_LogSetPriority(int category, SDL_LogPriority priority);
void SDL_Log(const char *fmt, ...);
void SDL_LogDebug(int category, const char *fmt, ...);
void SDL_LogWarn(int category, const char *fmt, ...);
void SDL_LogError(int category, const char *fmt, ...);

// ---- audio: only nortsong.c's converter, 8-bit samples to 16 ----
enum { AUDIO_S8 = 0x8008, AUDIO_S16SYS = 0x8010 };
typedef struct {
	int needed;
	Uint8 *buf;
	int len, len_cvt, len_mult;
	double len_ratio;
} SDL_AudioCVT;
int SDL_BuildAudioCVT(SDL_AudioCVT *cvt, Uint16 sfmt, Uint8 sch, int srate, Uint16 dfmt, Uint8 dch, int drate);
int SDL_ConvertAudio(SDL_AudioCVT *cvt);

#endif
