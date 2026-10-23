// tyr-host.c -- OpenTyrian built by gcc for the host (make tyrian-host): no
// display and no sound, only the clock. The reference for make
// test-tyrian, whose --timedemo frames the c4lc build must match.

#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>

int tyr_open_display(void) { return 0; }
void tyr_close_display(void) {}
// the data where it runs, or where -t says
const char *tyr_base(void) { return "./"; }
int tyr_has_sound(void) { return 0; }
uint32_t plat_ms(void) {
	struct timeval tv;
	gettimeofday(&tv, 0);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
void plat_sleep(uint32_t ms) { usleep(ms * 1000); }

// what gui.c and snd.c ask the machine: here, nothing is fitted
int __c4_info(void) { return 0; }
