/*
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) The OpenTyrian Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
// loudness-c4.c -- OpenTyrian's loudness.c on libjs's sound card
// (libjs/guest/snd.h), which is an AdLib and eight channels of samples, as
// Tyrian's own hardware was. The music player, lds_play.c, writes the OPL's
// registers (opl.h sends them here, not to opl.c's emulator), and its
// updates are stamped with the card's stream time at Tyrian's rate, 69.5 a
// second: tyr_audio_pump plays updates until the stream is a quarter of a
// second ahead, and the event loop and every delay call it (c4sdl.c). The
// sound effects play as they are, 11025 Hz, which nortsong.c then leaves
// at that rate (audioSampleRate), copied into 8 bits when they start.
//
// This is the unit that carries snd.h's functions.

#include "loudness.h"

#include "file.h"
#include "lds_play.h"
#include "logging.h"
#include "nortsong.h"
#include "opentyr.h"
#include "snd.h"

#include <stdlib.h>
#include <string.h>

int  tyr_has_sound(void);       // tyr-*.c: the card is there

enum {
	LDS_RATE2 = 139,            // twice the update rate: 69.5 Hz
	AHEAD_US = 250000,
	CHANNEL_COUNT = 8,
	CHANNEL_VOLUME_LEVELS = 8,
};

int audioSampleRate = 11025;

static bool music_stopped = true;
unsigned int song_playing = 0;

bool audio_disabled = false, music_disabled = false, samples_disabled = false;

static Uint8 musicVolume = 255;
static Uint8 sampleVolume = 255;

static long *songPositions = NULL;
static Uint16 songsCount = 0;

static Uint8 *chanbuf[CHANNEL_COUNT];
static size_t chancap[CHANNEL_COUNT];

static void load_song(unsigned int song_num);

// ---- the OPL, for lds_play.c (opl.h) ----

void tyr_opl_write(int reg, int val)
{
	if (!audio_disabled)
		snd_opl(reg, val);
}

// lds_play.c's opl_init: every register quiet, as a reset leaves them
void tyr_opl_init(void)
{
	if (audio_disabled)
		return;
	for (int r = 0x20; r <= 0xf5; ++r)
		snd_opl(r, (r >= 0x40 && r <= 0x55) ? 0x3f : 0);
	snd_opl(0x01, 0x20);
	snd_opl(0xbd, 0);
}

// every note off
static void opl_quiet(void)
{
	for (int ch = 0; ch < 9; ++ch)
		snd_opl(0xb0 + ch, 0);
}

bool init_audio(void)
{
	if (audio_disabled)
		return false;

	if (!tyr_has_sound())
	{
		audio_disabled = true;
		return false;
	}

	snd_reset();
	snd_clock(LDS_RATE2 * 10);   // a stream unit is 1/1390 s: an update is 20
	tyr_opl_init();

	return true;
}

// The music, kept a quarter of a second ahead of the card's clock
void tyr_audio_pump(void)
{
	int n = 0;

	if (audio_disabled || music_disabled || music_stopped)
		return;

	while (snd_ahead() < AHEAD_US && n++ < 64)
	{
		lds_update();
		snd_wait(20);
	}
}

void deinit_audio(void)
{
	if (audio_disabled)
		return;

	snd_stop(-1);
	opl_quiet();

	lds_free();
}

static void load_song(unsigned int song_num)  // FKA NortSong.loadSong
{
	const char *filename = "music.mus";

	File file = dataFileOpen(filename, "rb");
	if (file.error)
	{
		logFatal("Failed to open file '%s': %s", filename, fileGetError(&file));
		exit(EXIT_FAILURE);
	}

	static bool first = true;
	if (first)
	{
		first = false;

		songsCount = fileReadU16(&file);

		size_t songPositionsCount = songsCount + 1;
		songPositions = malloc(sizeof *songPositions * songPositionsCount);

		for (size_t i = 0; i < songsCount; ++i)
			songPositions[i] = fileReadU32(&file);

		songPositions[songsCount] = fileGetLength(&file);

		if (file.error)
			logError("Failed to read from file '%s': %s", filename, fileGetError(&file));
	}

	if (song_num >= songsCount)
	{
		logError("Attempted to load song %u, which does not exist.", song_num);
		return;
	}

	long position = songPositions[song_num];
	long endPosition = songPositions[song_num + 1];
	size_t size = endPosition > position ? endPosition - position : 0;

	fileSetPosition(&file, position);

	void *data = malloc(size);
	fileReadExactly(&file, data, size);

	if (file.error)
		logError("Failed to read from file '%s': %s", filename, fileGetError(&file));

	fileClose(&file);

	if (!lds_load(data, size))
		logError("Failed to load song %u.", song_num);

	free(data);
}

void play_song(unsigned int song_num)  // FKA NortSong.playSong
{
	if (audio_disabled)
		return;

	if (song_num != song_playing)
	{
		music_stopped = true;
		opl_quiet();

		load_song(song_num);

		song_playing = song_num;
	}

	music_stopped = false;
}

void restart_song(void)  // FKA Player.selectSong(1)
{
	if (audio_disabled)
		return;

	lds_rewind();

	music_stopped = false;
}

void stop_song(void)  // FKA Player.selectSong(0)
{
	if (audio_disabled)
		return;

	music_stopped = true;
	opl_quiet();
}

void fade_song(void)  // FKA Player.selectSong($C001)
{
	if (audio_disabled)
		return;

	lds_fade(1);
}

// The card mixes its OPL at its own level, so the music's volume is the
// game's to set only for the samples
void set_volume(Uint8 musicVolume_, Uint8 sampleVolume_)  // FKA NortSong.setVol and Player.setVol
{
	if (audio_disabled)
		return;

	musicVolume = musicVolume_;
	sampleVolume = sampleVolume_;
}

// A sample on channel chan: 16 bits in, the card's unsigned 8 out, at
// vol of eight levels of the samples' volume
void multiSamplePlay(const Sint16 *samples, size_t sampleCount, Uint8 chan, Uint8 vol)  // FKA Player.multiSamplePlay
{
	if (audio_disabled || samples_disabled || chan >= CHANNEL_COUNT || sampleCount == 0)
		return;

	if (chancap[chan] < sampleCount)
	{
		free(chanbuf[chan]);
		chanbuf[chan] = malloc(sampleCount);
		chancap[chan] = chanbuf[chan] ? sampleCount : 0;
		if (!chanbuf[chan])
			return;
	}
	for (size_t i = 0; i < sampleCount; ++i)
		chanbuf[chan][i] = (Uint8) ((samples[i] >> 8) + 128);

	int v = sampleVolume * (vol + 1) / CHANNEL_VOLUME_LEVELS;
	snd_play(chan, (char *) chanbuf[chan], (int) sampleCount, audioSampleRate, v | (v << 8));
}
