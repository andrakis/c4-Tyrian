// tyr-bare.c -- OpenTyrian on the bare machine and under C4DOS: the display
// and the sound card are the game's to take. gui.h and snd.h for their
// declarations only: video-c4.c and loudness-c4.c carry their functions.

#include <stdint.h>
#define C4IX_SHARED 1
#include "gui.h"
#include "snd.h"
#undef C4IX_SHARED

int tyr_open_display(void) { return gui_present(); }
void tyr_close_display(void) {}
// the games' disk has the data in tyrian/data (build-images.sh)
const char *tyr_base(void) { return "tyrian/"; }
int tyr_has_sound(void) { return snd_present(); }
uint32_t plat_ms(void) { return __time(); }
void plat_sleep(uint32_t ms) { if (ms) __c4_usleep(ms * 1000); }
