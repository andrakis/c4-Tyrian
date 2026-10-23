// tyr-c4ix.c -- OpenTyrian on C4IX: the display is the desktop's, which
// gives it way until the game ends (include/display.h); the clock and the
// sound card are the machine's. gui.h and snd.h for their declarations
// only: video-c4.c and loudness-c4.c carry their functions.

#include <stdint.h>
#define C4IX_SHARED 1
#include "gui.h"
#include "snd.h"
#undef C4IX_SHARED
#include "display.h"

int tyr_open_display(void) {
	if (!gui_present()) return 0;
	return udisplay_take(0);
}
void tyr_close_display(void) { udisplay_give(); }
// the game in /usr/games/tyrian, its data in data/ there (c4ix.vfs.txt)
const char *tyr_base(void) { return "/usr/games/tyrian/"; }
int tyr_has_sound(void) { return snd_present(); }
uint32_t plat_ms(void) { return __time(); }
void plat_sleep(uint32_t ms) { if (ms) __c4_usleep(ms * 1000); }
