// c4sdl.c -- c4/SDL.h's functions over libjs's display (libjs/guest/gui.h):
// surfaces, the event queue made from the display's events, the clock,
// the log, and a joystick that is never there. video-c4.c attaches the
// display, shows the frames and carries gui.h's functions (this has its
// declarations only); loudness-c4.c is the sound card; tyrian-sum.c the
// clock, virtual while a timedemo runs.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "SDL.h"
#define C4IX_SHARED 1
#include "gui.h"
#undef C4IX_SHARED

int  tyr_poll(int *ev);         // video-c4.c: the display's next event
void tyr_audio_pump(void);      // loudness-c4.c: keep the music ahead
Uint32 tyr_ticks(void);         // tyrian-sum.c
void tyr_delay(Uint32 ms);

// ---- small things ----
size_t SDL_strlcpy(char *dst, const char *src, size_t size) {
	size_t n = strlen(src);
	if (size) {
		size_t k = n < size - 1 ? n : size - 1;
		memcpy(dst, src, k);
		dst[k] = 0;
	}
	return n;
}
int  SDL_Init(Uint32 flags) { (void) flags; return 0; }
int  SDL_InitSubSystem(Uint32 flags) { (void) flags; return 0; }
void SDL_QuitSubSystem(Uint32 flags) { (void) flags; }
void SDL_Quit(void) {}
const char *SDL_GetError(void) { return "not on this machine"; }
SDL_bool SDL_SetHint(const char *name, const char *value) { (void) name; (void) value; return SDL_FALSE; }
// where the game is, which the platform says (tyr-c4ix.c, tyr-bare.c,
// tyr-host.c): its data is in "data" there, as a self-contained
// OpenTyrian's is (file.c, findDataFiles)
const char *tyr_base(void);
char *SDL_GetBasePath(void) {
	char *p = malloc(strlen(tyr_base()) + 1);
	if (p) strcpy(p, tyr_base());
	return p;
}
int SDL_ShowSimpleMessageBox(Uint32 flags, const char *title, const char *message, void *window) {
	(void) flags; (void) window;
	printf("%s: %s\n", title, message);
	return 0;
}
int SDL_GetNumVideoDisplays(void) { return 1; }
int SDL_ShowCursor(int toggle) { (void) toggle; return 0; }
int SDL_SetRelativeMouseMode(SDL_bool on) { (void) on; return 0; }

// ---- time: the machine's (tyrian-sum.c) ----
Uint32 SDL_GetTicks(void) { return tyr_ticks(); }
void   SDL_Delay(Uint32 ms) {
	tyr_audio_pump();
	tyr_delay(ms);
}

// ---- surfaces ----
static SDL_PixelFormat format8 = { 8, 1 };
SDL_Surface *SDL_CreateRGBSurface(Uint32 flags, int w, int h, int depth, Uint32 rm, Uint32 gm, Uint32 bm, Uint32 am) {
	SDL_Surface *s;
	(void) flags; (void) rm; (void) gm; (void) bm; (void) am;
	if (depth != 8) return NULL;
	if (!(s = calloc(1, sizeof(SDL_Surface)))) return NULL;
	s->format = &format8;
	s->w = w;
	s->h = h;
	s->pitch = w;
	if (!(s->pixels = calloc(w, h))) { free(s); return NULL; }
	return s;
}
void SDL_FreeSurface(SDL_Surface *s) {
	if (!s) return;
	free(s->pixels);
	free(s);
}
int SDL_FillRect(SDL_Surface *dst, const SDL_Rect *rect, Uint32 color) {
	int x0 = 0, y0 = 0, x1 = dst->w, y1 = dst->h, y;
	if (rect) {
		x0 = rect->x; y0 = rect->y; x1 = rect->x + rect->w; y1 = rect->y + rect->h;
		if (x0 < 0) x0 = 0;
		if (y0 < 0) y0 = 0;
		if (x1 > dst->w) x1 = dst->w;
		if (y1 > dst->h) y1 = dst->h;
	}
	for (y = y0; y < y1; y++)
		if (x1 > x0) memset((Uint8 *) dst->pixels + y * dst->pitch + x0, color, x1 - x0);
	return 0;
}
// the whole of one 8-bit surface onto another of its size: game_menu.c's one use
int SDL_BlitSurface(SDL_Surface *src, const SDL_Rect *srect, SDL_Surface *dst, SDL_Rect *drect) {
	int y, w, h;
	(void) srect; (void) drect;
	w = src->w < dst->w ? src->w : dst->w;
	h = src->h < dst->h ? src->h : dst->h;
	for (y = 0; y < h; y++)
		memcpy((Uint8 *) dst->pixels + y * dst->pitch, (Uint8 *) src->pixels + y * src->pitch, w);
	return 0;
}
Uint32 SDL_MapRGB(const SDL_PixelFormat *format, Uint8 r, Uint8 g, Uint8 b) {
	(void) format;
	return ((Uint32) r << 16) | ((Uint32) g << 8) | b;
}

// ---- scancodes, and their names for the config file ----
static const char *const sc_names[] = {
	"", "", "", "", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
	"P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
	"Return", "Escape", "Backspace", "Tab", "Space", "-", "=", "[", "]", "\\", "#", ";", "'",
	"`", ",", ".", "/", "CapsLock",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
	"PrintScreen", "ScrollLock", "Pause", "Insert", "Home", "PageUp", "Delete", "End",
	"PageDown", "Right", "Left", "Down", "Up", "Numlock", "Keypad /", "Keypad *",
	"Keypad -", "Keypad +", "Keypad Enter", "Keypad 1", "Keypad 2", "Keypad 3", "Keypad 4",
	"Keypad 5", "Keypad 6", "Keypad 7", "Keypad 8", "Keypad 9", "Keypad 0",
};
static const char *sc_name2(SDL_Scancode sc) {
	switch (sc) {
	case SDL_SCANCODE_LCTRL: return "Left Ctrl";
	case SDL_SCANCODE_LSHIFT: return "Left Shift";
	case SDL_SCANCODE_LALT: return "Left Alt";
	case SDL_SCANCODE_RCTRL: return "Right Ctrl";
	case SDL_SCANCODE_RSHIFT: return "Right Shift";
	case SDL_SCANCODE_RALT: return "Right Alt";
	}
	return "";
}
const char *SDL_GetScancodeName(SDL_Scancode sc) {
	if (sc >= 0 && sc < (int) (sizeof sc_names / sizeof sc_names[0])) return sc_names[sc];
	return sc_name2(sc);
}
static int name_eq(const char *a, const char *b) {
	while (*a && *b) {
		int x = *a >= 'a' && *a <= 'z' ? *a - 32 : *a;
		int y = *b >= 'a' && *b <= 'z' ? *b - 32 : *b;
		if (x != y) return 0;
		a++; b++;
	}
	return *a == *b;
}
SDL_Scancode SDL_GetScancodeFromName(const char *name) {
	int i;
	if (!name || !*name) return SDL_SCANCODE_UNKNOWN;
	for (i = 1; i < (int) (sizeof sc_names / sizeof sc_names[0]); i++)
		if (sc_names[i][0] && name_eq(name, sc_names[i])) return i;
	for (i = SDL_SCANCODE_LCTRL; i <= SDL_SCANCODE_RALT; i++)
		if (name_eq(name, sc_name2(i))) return i;
	return SDL_SCANCODE_UNKNOWN;
}

// a browser keyCode as SDL's scancode (Wolf3D's table, plat-gui.c)
static int scancode(int code) {
	if (code >= 'A' && code <= 'Z') return SDL_SCANCODE_A + code - 'A';
	if (code >= '1' && code <= '9') return SDL_SCANCODE_1 + code - '1';
	if (code == '0') return SDL_SCANCODE_0;
	if (code >= 112 && code <= 123) return SDL_SCANCODE_F1 + code - 112;
	switch (code) {
	case 13: return SDL_SCANCODE_RETURN;
	case 27: return SDL_SCANCODE_ESCAPE;
	case 8: return SDL_SCANCODE_BACKSPACE;
	case 9: return SDL_SCANCODE_TAB;
	case 32: return SDL_SCANCODE_SPACE;
	case 189: case 173: return SDL_SCANCODE_MINUS;
	case 187: case 61: return SDL_SCANCODE_EQUALS;
	case 219: return SDL_SCANCODE_LEFTBRACKET;
	case 221: return SDL_SCANCODE_RIGHTBRACKET;
	case 220: return SDL_SCANCODE_BACKSLASH;
	case 186: case 59: return SDL_SCANCODE_SEMICOLON;
	case 222: return SDL_SCANCODE_APOSTROPHE;
	case 192: return SDL_SCANCODE_GRAVE;
	case 188: return SDL_SCANCODE_COMMA;
	case 190: return SDL_SCANCODE_PERIOD;
	case 191: return SDL_SCANCODE_SLASH;
	case 20: return SDL_SCANCODE_CAPSLOCK;
	case 145: return SDL_SCANCODE_SCROLLLOCK;
	case 19: return SDL_SCANCODE_PAUSE;
	case 45: return SDL_SCANCODE_INSERT;
	case 46: return SDL_SCANCODE_DELETE;
	case 36: return SDL_SCANCODE_HOME;
	case 35: return SDL_SCANCODE_END;
	case 33: return SDL_SCANCODE_PAGEUP;
	case 34: return SDL_SCANCODE_PAGEDOWN;
	case 37: return SDL_SCANCODE_LEFT;
	case 38: return SDL_SCANCODE_UP;
	case 39: return SDL_SCANCODE_RIGHT;
	case 40: return SDL_SCANCODE_DOWN;
	case 16: return SDL_SCANCODE_LSHIFT;
	case 17: return SDL_SCANCODE_LCTRL;
	case 18: return SDL_SCANCODE_LALT;
	}
	return SDL_SCANCODE_UNKNOWN;
}
// ... and as SDL's key code: the character it types, lower case, or SDL's
// own for the keys that type none
static SDL_Keycode keysym(int code, int ch) {
	if (code >= 'A' && code <= 'Z') return code + 32;
	if (code >= '0' && code <= '9') return code;
	switch (code) {
	case 13: return SDLK_RETURN;
	case 27: return SDLK_ESCAPE;
	case 8: return SDLK_BACKSPACE;
	case 9: return SDLK_TAB;
	case 32: return SDLK_SPACE;
	case 221: return SDLK_RIGHTBRACKET;
	}
	if (ch >= 33 && ch < 127) return ch >= 'A' && ch <= 'Z' ? ch + 32 : ch;
	return 0x40000000 | scancode(code);
}
static Uint16 keymod(int mods) {
	return (mods & 1 ? KMOD_LSHIFT : 0) | (mods & 2 ? KMOD_LCTRL : 0) | (mods & 4 ? KMOD_LALT : 0);
}

// ---- the event queue ----
enum { QLEN = 64 };
static SDL_Event q[QLEN];
static int qh, qt;
static int texting;
static Uint8 down[SDL_NUM_SCANCODES];
static int mx = 320, my = 240, mbuttons;

static void push(SDL_Event *e) {
	if ((qt + 1) % QLEN == qh) return;      // full: dropped
	q[qt] = *e;
	qt = (qt + 1) % QLEN;
}
int SDL_PushEvent(SDL_Event *ev) { push(ev); return 1; }
void SDL_StartTextInput(void) { texting = 1; }
void SDL_StopTextInput(void) { texting = 0; }

// the display's events, as SDL's
static void gather(void) {
	int ev[4], sc;
	SDL_Event e;
	while (tyr_poll(ev)) {
		memset(&e, 0, sizeof e);
		if (ev[0] == GUI_EV_KEYDOWN || ev[0] == GUI_EV_KEYUP) {
			sc = scancode(ev[1]);
			e.type = ev[0] == GUI_EV_KEYDOWN ? SDL_KEYDOWN : SDL_KEYUP;
			e.key.state = ev[0] == GUI_EV_KEYDOWN ? SDL_PRESSED : SDL_RELEASED;
			e.key.repeat = ev[0] == GUI_EV_KEYDOWN && down[sc];
			e.key.keysym.scancode = sc;
			e.key.keysym.sym = keysym(ev[1], ev[2]);
			e.key.keysym.mod = keymod(ev[3]);
			down[sc] = ev[0] == GUI_EV_KEYDOWN;
			push(&e);
			if (ev[0] == GUI_EV_KEYDOWN && texting && ev[2] >= 32 && ev[2] < 127 && !(ev[3] & 6)) {
				memset(&e, 0, sizeof e);
				e.type = SDL_TEXTINPUT;
				e.text.text[0] = (char) ev[2];
				push(&e);
			}
		} else if (ev[0] == GUI_EV_MOVE) {
			e.type = SDL_MOUSEMOTION;
			e.motion.x = ev[1]; e.motion.y = ev[2];
			e.motion.xrel = ev[1] - mx; e.motion.yrel = ev[2] - my;
			e.motion.state = mbuttons;
			mx = ev[1]; my = ev[2];
			push(&e);
		} else if (ev[0] == GUI_EV_REL) {
			// the mouse locked to the display: its motion, which the
			// display's edge does not stop, and a pointer kept inside it
			e.type = SDL_MOUSEMOTION;
			mx += ev[1]; my += ev[2];
			if (mx < 0) mx = 0;
			if (mx > 639) mx = 639;
			if (my < 0) my = 0;
			if (my > 479) my = 479;
			e.motion.x = mx; e.motion.y = my;
			e.motion.xrel = ev[1]; e.motion.yrel = ev[2];
			e.motion.state = mbuttons;
			push(&e);
		} else if (ev[0] == GUI_EV_DOWN || ev[0] == GUI_EV_UP) {
			e.type = ev[0] == GUI_EV_DOWN ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
			e.button.button = ev[3] == 2 ? SDL_BUTTON_RIGHT : ev[3] == 1 ? SDL_BUTTON_MIDDLE : SDL_BUTTON_LEFT;
			e.button.state = ev[0] == GUI_EV_DOWN ? SDL_PRESSED : SDL_RELEASED;
			e.button.clicks = 1;
			e.button.x = ev[1]; e.button.y = ev[2];
			if (ev[0] == GUI_EV_DOWN) mbuttons |= SDL_BUTTON(e.button.button);
			else mbuttons &= ~SDL_BUTTON(e.button.button);
			mx = ev[1]; my = ev[2];
			push(&e);
		} else if (ev[0] == GUI_EV_FOCUS) {
			e.type = SDL_WINDOWEVENT;
			e.window.event = ev[1] ? SDL_WINDOWEVENT_FOCUS_GAINED : SDL_WINDOWEVENT_FOCUS_LOST;
			push(&e);
		}
	}
}

int SDL_PollEvent(SDL_Event *ev) {
	tyr_audio_pump();
	gather();
	if (qh == qt) return 0;
	*ev = q[qh];
	qh = (qh + 1) % QLEN;
	return 1;
}

// ---- joysticks: there are none ----
int  SDL_NumJoysticks(void) { return 0; }
SDL_Joystick *SDL_JoystickOpen(int i) { (void) i; return NULL; }
void SDL_JoystickClose(SDL_Joystick *j) { (void) j; }
const char *SDL_JoystickName(SDL_Joystick *j) { (void) j; return ""; }
int  SDL_JoystickNumAxes(SDL_Joystick *j) { (void) j; return 0; }
int  SDL_JoystickNumButtons(SDL_Joystick *j) { (void) j; return 0; }
int  SDL_JoystickNumHats(SDL_Joystick *j) { (void) j; return 0; }
Sint16 SDL_JoystickGetAxis(SDL_Joystick *j, int axis) { (void) j; (void) axis; return 0; }
Uint8 SDL_JoystickGetButton(SDL_Joystick *j, int button) { (void) j; (void) button; return 0; }
Uint8 SDL_JoystickGetHat(SDL_Joystick *j, int hat) { (void) j; (void) hat; return 0; }
void SDL_JoystickUpdate(void) {}
int  SDL_JoystickEventState(int state) { (void) state; return 0; }

// ---- the log: to stdout, which on C4IX is the Terminal ----
static void log_out(void *userdata, int category, SDL_LogPriority priority, const char *message) {
	(void) userdata; (void) category; (void) priority;
	fputs(message, stdout);
}
void SDL_LogGetOutputFunction(SDL_LogOutputFunction *callback, void **userdata) {
	*callback = log_out;
	*userdata = NULL;
}
void SDL_LogSetPriority(int category, SDL_LogPriority priority) { (void) category; (void) priority; }
static void vlog(const char *fmt, va_list ap) {
	vprintf(fmt, ap);
	printf("\n");
}
void SDL_Log(const char *fmt, ...) { va_list ap; va_start(ap, fmt); vlog(fmt, ap); va_end(ap); }
void SDL_LogDebug(int c, const char *fmt, ...) { va_list ap; (void) c; va_start(ap, fmt); vlog(fmt, ap); va_end(ap); }
void SDL_LogWarn(int c, const char *fmt, ...) { va_list ap; (void) c; va_start(ap, fmt); vlog(fmt, ap); va_end(ap); }
void SDL_LogError(int c, const char *fmt, ...) { va_list ap; (void) c; va_start(ap, fmt); vlog(fmt, ap); va_end(ap); }

// ---- audio: nortsong.c's one conversion, signed 8-bit to 16 at the same rate ----
int SDL_BuildAudioCVT(SDL_AudioCVT *cvt, Uint16 sfmt, Uint8 sch, int srate, Uint16 dfmt, Uint8 dch, int drate) {
	if (sfmt != AUDIO_S8 || dfmt != AUDIO_S16SYS || sch != 1 || dch != 1 || srate != drate) return -1;
	memset(cvt, 0, sizeof *cvt);
	cvt->needed = 1;
	cvt->len_mult = 2;
	cvt->len_ratio = 2.0;
	return 1;
}
int SDL_ConvertAudio(SDL_AudioCVT *cvt) {
	int i;
	Sint16 *out = (Sint16 *) cvt->buf;
	Sint8 *in = (Sint8 *) cvt->buf;
	// from the end, so nothing is read after it is written
	for (i = cvt->len - 1; i >= 0; i--) out[i] = (Sint16) (in[i] * 256);
	cvt->len_cvt = cvt->len * 2;
	return 0;
}
