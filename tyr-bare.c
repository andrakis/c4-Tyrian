// tyr-bare.c -- OpenTyrian on the bare machine and under C4DOS: the display
// and the sound card are the game's to take. gui.h and snd.h for their
// declarations only: video-c4.c and loudness-c4.c carry their functions.
//
// Under grin, C4DOS's graphical shell, the display is grin's, and grin
// publishes a table in C4DOS's slot 25 while it has it (c4's
// docs/grin-plan.md 3.6): then the game plays in a window of grin's, the
// window video-c4.c asks for when the display is not the game's, steered
// by the mouse locked to it with a click, and grin's loop runs in its
// polls.

#include <stdint.h>
#include <stdlib.h>
#define C4IX_SHARED 1
#include "gui.h"
#include "snd.h"
#undef C4IX_SHARED

// grin's table (c4's src/grin/gkit.h): its first eight words
struct grin_table {
	int magic, version, gen;
	void *env;
	void (*hello)(void);
	int  (*fwin_open)(char *title, int w, int h, int flags);
	void (*fwin_frame)(char *pixels, int w, int h, int pitch, int *pal);
	int  (*fwin_poll)(int *ev);
};
extern int *__c4dos_api;                // bare.c's: C4DOS's table, 0 on the bare machine

static struct grin_table *grin(void) {
	struct grin_table *t;
	if (!__c4dos_api || __c4dos_api[0] != 4404292 || __c4dos_api[5] < 4) return 0;
	t = (struct grin_table *) __c4dos_api[25];
	return t && t->magic == 1196575054 && t->version >= 2 ? t : 0;
}

static struct grin_table *g;

int tyr_open_display(void) { return !grin() && gui_present(); }
void tyr_close_display(void) {}
// grin's window (flags 1: the mouse steers)
int tyr_win_open(void) { return (g = grin()) != 0 && g->fwin_open("Tyrian", 320, 200, 1); }
void tyr_win_draw(uint8_t *pixels, int pitch, uint32_t *palette) {
	if (g) g->fwin_frame((char *) pixels, 320, 200, pitch, (int *) palette);
}
// its next event, the mouse's in the display's 640x480 as c4sdl.c reads it
int tyr_win_poll(int *ev) {
	int k;
	if (!g) return 0;
	// the window closed: the game ends, as when it is quit
	if ((k = g->fwin_poll(ev)) < 0) exit(0);
	if (k && (ev[0] == GUI_EV_MOVE || ev[0] == GUI_EV_DOWN || ev[0] == GUI_EV_UP || ev[0] == GUI_EV_REL)) {
		ev[1] = ev[1] * 2;
		ev[2] = ev[2] * 12 / 5;
	}
	return k;
}
void tyr_win_close(void) {}
// the games' disk has the data in tyrian/data (build-images.sh)
const char *tyr_base(void) { return "tyrian/"; }
int tyr_has_sound(void) { return snd_present(); }
uint32_t plat_ms(void) { return __time(); }
void plat_sleep(uint32_t ms) { if (ms) __c4_usleep(ms * 1000); }
