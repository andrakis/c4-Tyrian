// tyrian-sum.c -- what every platform of this port shares: the clock, and
// --timedemo N, which plays the game's demo N (demo.N on the disk) from the
// title screen at once and exits when it ends, printing
//
//   timedemo: demo 1: 2711 frames, checksum 1a2b3c4d
//
// a checksum of every frame shown, its palette with it. While timing the
// clock is virtual, as Wolf3D's and Doom's are -- each reading a
// millisecond later, and a delay adding its wait -- so the frames are the
// same on any machine, and the gcc and c4lc builds must print the same
// line (make test-tyrian). There is no display and no sound then.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "palette.h"

Uint32 plat_ms(void);           // tyr-c4ix.c, tyr-bare.c, tyr-host.c
void   plat_sleep(Uint32 ms);

static int timing;
static int tracing;              // --trace: each frame's checksum as it goes
static int rowsframe;            // --rows N: and frame N row by row
static int demo;
static int titles;
static int frames;
static Uint32 sum;
static Uint32 now;

static void report(void) {
	printf("timedemo: demo %d: %d frames, checksum %08x\n", demo, frames, (unsigned) sum);
}

// main's first act: --timedemo N taken off the command line
void tyr_start(int *argc, char **argv) {
	int i, j;
	for (i = 1; i < *argc; i++) {
		if (strcmp(argv[i], "--trace") != 0) continue;
		tracing = 1;
		for (j = i; j + 1 <= *argc; j++) argv[j] = argv[j + 1];
		*argc = *argc - 1;
		break;
	}
	for (i = 1; i + 1 < *argc; i++) {
		if (strcmp(argv[i], "--rows") != 0) continue;
		rowsframe = atoi(argv[i + 1]);
		for (j = i; j + 2 <= *argc; j++) argv[j] = argv[j + 2];
		*argc = *argc - 2;
		break;
	}
	for (i = 1; i + 1 < *argc; i++) {
		if (strcmp(argv[i], "--timedemo") != 0) continue;
		demo = atoi(argv[i + 1]);
		if (demo < 1 || demo > 5) demo = 1;
		timing = 1;
		for (j = i; j + 2 <= *argc; j++) argv[j] = argv[j + 2];
		*argc = *argc - 2;
		atexit(report);
		return;
	}
}

int tyr_headless(void) { return timing; }
int tyr_timedemo(void) { return demo; }

// the title screen, waiting: the first time it starts the demo, the second
// time the demo has ended
int tyr_title(void) {
	if (!timing) return 0;
	if (titles++ == 0) return 1;
	exit(0);
	return 0;
}

void tyr_frame(SDL_Surface *s) {
	int i, y;
	Uint32 h;
	Uint8 *p;
	if (!timing) return;
	h = sum;
	for (i = 0; i < 256; i++) h = h * 31 + rgb_palette[i];
	for (y = 0; y < s->h; y++) {
		p = (Uint8 *) s->pixels + y * s->pitch;
		for (i = 0; i < s->w; i += 4)
			h = h * 31 + (Uint32) (p[i] | (p[i + 1] << 8) | (p[i + 2] << 16) | ((Uint32) p[i + 3] << 24));
	}
	sum = h;
	frames++;
	if (tracing) printf("frame %d %08x\n", frames, (unsigned) h);
	// --rows N: frame N row by row
	if (rowsframe == frames) {
		for (y = 0; y < s->h; y++) {
			p = (Uint8 *) s->pixels + y * s->pitch;
			h = 0;
			for (i = 0; i < s->w; i++) h = h * 31 + p[i];
			printf("row %d %08x\n", y, (unsigned) h);
		}
		for (i = 0; i < 256; i++) printf("pal %d %06x\n", i, (unsigned) rgb_palette[i]);
	}
}

Uint32 tyr_ticks(void) {
	if (timing) return ++now;
	return plat_ms();
}

void tyr_delay(Uint32 ms) {
	if (timing) { now += ms; return; }
	plat_sleep(ms);
}
