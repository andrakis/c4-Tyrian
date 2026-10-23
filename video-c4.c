// video-c4.c -- OpenTyrian's video.c and video_scale.c on libjs's display
// (libjs/guest/gui.h). The game draws into 320x200 8-bit surfaces, as it
// always has; JE_showVGA sends VGAScreen as GUI_BLIT8 with the palette,
// whose rgb_palette SDL_MapRGB makes 0x00RRGGBB (c4sdl.c), and the host
// scales it to 640x480: 2x across and 2.4x down, the 4:3 a VGA monitor
// showed. There is one scaler, the host's, and no window to resize.
//
// On C4IX the desktop has the display, and gives it way until the game
// ends (tyr-c4ix.c) -- or, with --window or when a program that does not
// give way has it, the game plays in the Terminal window it was started
// from; on the bare machine and C4DOS it is the game's
// (tyr-bare.c); the host's build has none (tyr-host.c). A timedemo runs
// without it (tyrian-sum.c). This is the unit that carries gui.h's
// functions; the others take its declarations only.

#include "video.h"

#include "keyboard.h"
#include "palette.h"
#include "video_scale.h"
#include "gui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int  tyr_open_display(void);    // tyr-*.c: the display is there and ours
void tyr_close_display(void);
int  tyr_headless(void);        // tyrian-sum.c: a timedemo
int  tyr_windowed(void);        //   and --window
int  tyr_win_open(void);        // tyr-*.c: a window instead (C4IX's)
void tyr_win_draw(Uint8 *pixels, int pitch, Uint32 *palette);
int  tyr_win_poll(int *ev);
void tyr_win_close(void);

enum { W = 640, H = 480, RING = 65536 };

const char *const scaling_mode_names[ScalingMode_MAX] = {
	"Center",
	"Integer",
	"Fit 8:5",
	"Fit 4:3",
};

int fullscreen_display;
ScalingMode scaling_mode = SCALE_ASPECT_4_3;

SDL_Surface *VGAScreen, *VGAScreenSeg;
SDL_Surface *VGAScreen2;
SDL_Surface *game_screen;

SDL_Window *main_window = NULL;
static SDL_PixelFormat format32 = { 32, 4 };
SDL_PixelFormat *main_window_tex_format = &format32;

// video_scale.c's: one scaler, the host's
uint scaler = 0;
const struct Scalers scalers[] = {
	{ 640, 480, NULL, NULL, "None" },
};
const uint scalers_count = 1;
void set_scaler_by_name(const char *name) { (void) name; }

static int on;
static int win;
static int *ring;

// the display's next event, for c4sdl.c's queue; or the window's, in the
// display's form
int tyr_poll(int *ev) {
	if (win) return tyr_win_poll(ev);
	return on && gui_poll(ev);
}

static void close_win(void) { if (win) tyr_win_close(); win = 0; }
static int open_win(void) {
	if (!tyr_win_open()) return 0;
	win = 1;
	atexit(close_win);
	return 1;
}

static void close_display(void) {
	if (!on) return;
	gui_cursor(GUI_PTR_ARROW);
	gui_detach();
	tyr_close_display();
	on = 0;
}

void init_video(void)
{
	VGAScreen = VGAScreenSeg = SDL_CreateRGBSurface(0, vga_width, vga_height, 8, 0, 0, 0, 0);
	VGAScreen2 = SDL_CreateRGBSurface(0, vga_width, vga_height, 8, 0, 0, 0, 0);
	game_screen = SDL_CreateRGBSurface(0, vga_width, vga_height, 8, 0, 0, 0, 0);
	if (!VGAScreen || !VGAScreen2 || !game_screen) {
		printf("opentyrian: out of memory for the screens\n");
		exit(EXIT_FAILURE);
	}
	JE_clr256(VGAScreen);

	if (tyr_headless()) return;
	if (tyr_windowed() && open_win()) return;
	if (!tyr_open_display()) {
		if (open_win()) { printf("opentyrian: a program has the display; playing in the window\n"); return; }
		printf("opentyrian: no display to draw on\n");
		return;
	}
	if (!(ring = malloc(RING)) || !gui_attach(ring, RING, W, H)) { tyr_close_display(); return; }
	gui_events(GUI_M_KEYS | GUI_M_MOVE | GUI_M_BUTTONS | GUI_M_WINDOW | GUI_M_REL);
	// the game draws its own pointer, and the mouse steers the ship: locked
	// to the display from a click on it (Esc lets it go), so it cannot
	// wander off it
	gui_cursor(GUI_PTR_LOCK);
	gui_clear(0x000000);
	gui_show();
	on = 1;
	atexit(close_display);
}

void deinit_video(void)
{
	close_display();
	SDL_FreeSurface(VGAScreenSeg);
	SDL_FreeSurface(VGAScreen2);
	SDL_FreeSurface(game_screen);
}

void video_on_win_resize(void) {}
void reinit_fullscreen(int new_display) { fullscreen_display = new_display; }
void toggle_fullscreen(void) {}
bool init_scaler(unsigned int new_scaler) { (void) new_scaler; scaler = 0; return true; }

bool set_scaling_mode_by_name(const char *name)
{
	for (int i = 0; i < ScalingMode_MAX; ++i)
	{
		if (strcmp(name, scaling_mode_names[i]) == 0)
		{
			scaling_mode = i;
			return true;
		}
	}
	return false;
}

void JE_clr256(SDL_Surface *screen)
{
	SDL_FillRect(screen, NULL, 0);
}

// the frame, to the checksum when timing (tyrian-sum.c) and to the display
void tyr_frame(SDL_Surface *s);

void JE_showVGA(void)
{
	tyr_frame(VGAScreen);
	if (win) { tyr_win_draw((Uint8 *) VGAScreen->pixels, VGAScreen->pitch, rgb_palette); return; }
	if (!on) return;
	gui_blit8(0, 0, vga_width, vga_height, (char *) VGAScreen->pixels, VGAScreen->pitch,
	          (int *) rgb_palette, W, H);
	gui_show();
}

// the display is 640x480, the screen 320x200
void mapScreenPointToWindow(Sint32 *inout_x, Sint32 *inout_y)
{
	*inout_x = *inout_x * W / vga_width;
	*inout_y = *inout_y * H / vga_height;
}

void mapWindowPointToScreen(Sint32 *inout_x, Sint32 *inout_y)
{
	*inout_x = *inout_x * vga_width / W;
	*inout_y = *inout_y * vga_height / H;
}

void scaleWindowDistanceToScreen(Sint32 *inout_x, Sint32 *inout_y)
{
	mapWindowPointToScreen(inout_x, inout_y);
}
