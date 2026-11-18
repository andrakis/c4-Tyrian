// acceltest.c -- c4's port: OpenTyrian's accelerators (c4/accel.mjs)
// against the loops they stand for, as sprite.c, backgrnd.c and
// tyrian-sum.c have them: the same pixels, on made-up sprites of both
// kinds, background tiles and screens, and the same checksums.
// test-tyrian runs it on c4's libjs.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "SDL.h"
#include "swar.h"
#include "c4accel.h"

int tyr_accel = -1;
int tyr_accel_on = 1;
int tyr_accel_base(void) { return tyr_accel = accel_find("tyrian"); }

unsigned seed = 12345;
int rnd(int n) { seed = seed * 1103515245 + 12345; return (int)((seed >> 8) % (unsigned)n); }

enum { W = 320, H = 200, BUFN = W * H, vga_height = 200 };
Uint8 *bufc, *bufa, *base, *spr, *tiles;
int fails, cases;

void fill(Uint8 *p, int n) { int i; for (i = 0; i < n; i++) p[i] = rnd(256); }
void check(char *what, int k) {
    cases++;
    if (memcmp(bufc, bufa, BUFN) != 0) {
        fails++;
        if (fails < 10) printf("accel: %s, case %d, differs\n", what, k);
    }
}
void clear() { int i; for (i = 0; i < 8; i++) base[rnd(BUFN)] = rnd(256); memcpy(bufc, base, BUFN); memcpy(bufa, base, BUFN); }

// sprite.c's blit_sprite, _blend, _hv_unsafe, _hv, _hv_blend and _dark:
// the one loop they share, each one's pixel as it has it
void spritec(Uint8 *buf, int x, int y, const Uint8 *data, unsigned size, unsigned width, int kind, Uint8 hue, Sint8 value, int black) {
    const Uint8 * const data_ul = data + size;
    unsigned int x_offset = 0;
    Uint8 *pixels = buf + (y * W) + x;
    const Uint8 * const pixels_ll = buf, * const pixels_ul = buf + (H * W);
    hue <<= 4;
    for (; data < data_ul; ++data) {
        switch (*data) {
        case 255: data++; pixels += *data; x_offset += *data; break;
        case 254: pixels += width - x_offset; x_offset = width; break;
        case 253: pixels++; x_offset++; break;
        default:
            if (pixels >= pixels_ul) return;
            if (pixels >= pixels_ll) {
                if (kind == TS_PLAIN) *pixels = *data;
                else if (kind == TS_BLEND) *pixels = (*data & 0xf0) | (((*pixels & 0x0f) + (*data & 0x0f)) / 2);
                else if (kind == TS_HV_UNSAFE) *pixels = hue | ((*data & 0x0f) + value);
                else if (kind == TS_HV || kind == TS_HV_BLEND) {
                    Uint8 temp_value = (*data & 0x0f) + value;
                    if (temp_value > 0xf)
                        temp_value = (temp_value >= 0x1f) ? 0x0 : 0xf;
                    if (kind == TS_HV) *pixels = hue | temp_value;
                    else *pixels = hue | (((*pixels & 0x0f) + temp_value) / 2);
                }
                else *pixels = black ? 0x00 : ((*pixels & 0xf0) | ((*pixels & 0x0f) / 2));
            }
            pixels++; x_offset++;
            break;
        }
        if (x_offset >= width) { pixels += W - x_offset; x_offset = 0; }
    }
}
int makesprite(unsigned width) {
    int n = 0, len = 1 + rnd(600), r;
    while (n < len) {
        r = rnd(10);
        if (r == 0) { spr[n++] = 255; spr[n++] = rnd(width + 3); }
        else if (r == 1) spr[n++] = 254;
        else if (r == 2) spr[n++] = 253;
        else spr[n++] = rnd(253);
    }
    return n;
}
void test_sprite() {
    int k, kind, x, y, size, black;
    unsigned width;
    Uint8 hue;
    Sint8 value;
    for (k = 0; k < 1800; k++) {
        clear();
        kind = k % 6; width = 1 + rnd(40); size = makesprite(width);
        x = rnd(W); y = rnd(H + 40) - 30; hue = rnd(256); value = rnd(48) - 24; black = rnd(2);
        spritec(bufc, x, y, spr, size, width, kind, hue, value, black);
        tyr_sprite(spr, size, width, bufa + y * W + x, bufa, bufa + H * W, W, kind,
                   (Uint8)(hue << 4), kind == TS_DARK ? black : value);
        check("sprite", k);
    }
}

// blit_sprite2, _blend, _darken and _filter, which step a row by the
// screen's pitch less 12, and _clip and _filter_clip
void sprite2c(Uint8 *buf, int x, int y, const Uint8 *data, int kind, Uint8 filter) {
    Uint8 *pixels = buf + (y * W) + x;
    const Uint8 * const pixels_ll = buf, * const pixels_ul = buf + (H * W);
    for (; *data != 0x0f; ++data) {
        pixels += *data & 0x0f;
        unsigned int count = (*data & 0xf0) >> 4;
        if (count == 0) pixels += W - 12;
        else while (count--) {
            ++data;
            if (pixels >= pixels_ul) return;
            if (pixels >= pixels_ll) {
                if (kind == TS_2) *pixels = *data;
                else if (kind == TS_2_BLEND) *pixels = (((*data & 0x0f) + (*pixels & 0x0f)) / 2) | (*data & 0xf0);
                else if (kind == TS_2_DARKEN) *pixels = ((*pixels & 0x0f) / 2) + (*pixels & 0xf0);
                else *pixels = filter | (*data & 0x0f);
            }
            ++pixels;
        }
    }
}
void sprite2clipc(Uint8 *buf, int x, int y, const Uint8 *data, int kind, Uint8 filter) {
    for (; *data != 0x0f; ++data) {
        if (y >= H) return;
        Uint8 skip_count = *data & 0x0f;
        Uint8 fill_count = (*data >> 4) & 0x0f;
        x += skip_count;
        if (fill_count == 0) { y += 1; x -= 12; }
        else if (y >= 0) {
            Uint8 *const pixel_row = buf + (y * W);
            do {
                ++data;
                if (x >= 0 && x < W) pixel_row[x] = kind == TS_2 ? *data : filter | (*data & 0x0f);
                x += 1;
            } while (--fill_count);
        } else { data += fill_count; x += fill_count; }
    }
}
void makesprite2() {
    int n = 0, rows = 1 + rnd(16), r, c, i;
    for (r = 0; r < rows; r++) {
        while (rnd(4)) {
            c = 1 + rnd(5);
            spr[n++] = (c << 4) | rnd(4);
            for (i = 0; i < c; i++) spr[n++] = rnd(256);
        }
        spr[n++] = rnd(4);    // a row's end: no pixels, a skip of 0 to 3
    }
    spr[n++] = 0x0f;
}
void test_sprite2() {
    int k, kind, x, y, clip;
    Uint8 filter;
    for (k = 0; k < 1600; k++) {
        clear();
        clip = k & 1; kind = TS_2 + (k >> 1) % 4;
        if (clip) kind = (k >> 1) & 1 ? TS_2_FILTER : TS_2;
        makesprite2();
        x = rnd(W + 20) - 10; y = rnd(H + 40) - 30; filter = rnd(16) << 4;
        if (!clip) { x = rnd(W - 12); }
        if (clip) {
            sprite2clipc(bufc, x, y, spr, kind, filter);
            tyr_sprite2c(spr, x, y, bufa, W, H, kind, filter);
        } else {
            sprite2c(bufc, x, y, spr, kind, filter);
            tyr_sprite2(spr, bufa + y * W + x, bufa, bufa + H * W, W, kind, filter);
        }
        check(clip ? "sprite2, clipped" : "sprite2", k);
    }
}

// backgrnd.c's rows of tiles, as it has them (SWAR8_LANES 4)
static void blitBackground1Row(SDL_Surface *dst, int_fast16_t x, int_fast16_t y, Uint8 **map)
{
	Uint8 *dst_pixels = dst->pixels;
	int dst_pitch = dst->pitch;
	int_fast16_t ty = 0;
	int_fast16_t th = 28;
	if (y < 0) { ty = -y; y = 0; }
	else if (y > 184 - th) { th = 184 - y; }
	size_t di = y * dst_pitch + x;
	for (; ty < th; ++ty)
	{
		for (int ti = 0; ti < 12; ++ti)
		{
			Uint8 *data = map[ti];
			data += ty * 24;
			memcpy(&dst_pixels[di], data, 24);
			di += 24;
		}
		di += dst_pitch - 12 * 24;
	}
}
static void blitBackgroundRow(SDL_Surface *dst, int_fast16_t x, int_fast16_t y, Uint8 **map)
{
	Uint8 *dst_pixels = dst->pixels;
	int dst_pitch = dst->pitch;
	int_fast16_t ty = 0;
	int_fast16_t th = 28;
	if (y < 0) { ty = -y; y = 0; }
	else if (y > 184 - th) { th = 184 - y; }
	size_t di = y * dst_pitch + x;
	for (; ty < th; ++ty)
	{
		for (int ti = 0; ti < 12; ++ti)
		{
			Uint8 *data = map[ti];
			if (data == NULL) { di += 24; continue; }
			data += ty * 24;
			for (int tx = 24; tx > 0; tx -= sizeof(Swar8))
			{
				Swar8 dstPixels = swar8Read(&dst_pixels[di]);
				Swar8 srcPixels = swar8Read(data);
				Swar8 bit7 = (srcPixels & SWAR8(0x7F)) + SWAR8(0x7F);
				Swar8 bit = (srcPixels | bit7) & SWAR8(0x80);
				Swar8 mask = bit | (bit - (bit >> 7));
				dstPixels ^= (dstPixels ^ srcPixels) & mask;
				swar8Write(&dst_pixels[di], dstPixels);
				data += sizeof(Swar8);
				di += sizeof(Swar8);
			}
		}
		di += dst_pitch - 12 * 24;
	}
}
static void blitBackgroundRowBlend(SDL_Surface *dst, int_fast16_t x, int_fast16_t y, Uint8 **map)
{
	Uint8 *dst_pixels = dst->pixels;
	int dst_pitch = dst->pitch;
	int_fast16_t ty = 0;
	int_fast16_t th = 28;
	if (y < 0) { ty = -y; y = 0; }
	else if (y > 184 - th) { th = 184 - y; }
	size_t di = y * dst_pitch + x;
	for (; ty < th; ++ty)
	{
		for (int ti = 0; ti < 12; ++ti)
		{
			Uint8 *data = map[ti];
			if (data == NULL) { di += 24; continue; }
			data += ty * 24;
			for (int tx = 24; tx > 0; tx -= sizeof(Swar8))
			{
				Swar8 dstPixels = swar8Read(&dst_pixels[di]);
				Swar8 srcPixels = swar8Read(data);
				Swar8 bit7 = (srcPixels & SWAR8(0x7F)) + SWAR8(0x7F);
				Swar8 bit = (srcPixels | bit7) & SWAR8(0x80);
				Swar8 mask = bit | (bit - (bit >> 7));
				const Swar8 huesMask = SWAR8(0xF0);
				Swar8 hues = srcPixels & huesMask;
				const Swar8 valuesMask = SWAR8(0x0F);
				Swar8 values = srcPixels & valuesMask;
				values += dstPixels & valuesMask;
				values = (values >> 1) & valuesMask;
				srcPixels = hues | values;
				dstPixels ^= (dstPixels ^ srcPixels) & mask;
				swar8Write(&dst_pixels[di], dstPixels);
				data += sizeof(Swar8);
				di += sizeof(Swar8);
			}
		}
		di += dst_pitch - 12 * 24;
	}
}
Uint8 *map[12];
void test_bgrow() {
    int k, kind, x, y, i;
    SDL_Surface s;
    memset(&s, 0, sizeof s);
    s.pitch = W; s.w = W; s.h = H;
    for (k = 0; k < 600; k++) {
        clear();
        kind = k % 3; x = rnd(24); y = rnd(184 + 28 + 16) - 28;
        for (i = 0; i < 12; i++) map[i] = kind != TB_OPAQUE && rnd(4) == 0 ? NULL : tiles + 672 * rnd(16);
        s.pixels = bufc;
        if (kind == TB_OPAQUE) blitBackground1Row(&s, x, y, map);
        else if (kind == TB_CLEAR) blitBackgroundRow(&s, x, y, map);
        else blitBackgroundRowBlend(&s, x, y, map);
        tyr_bgrow(bufa, W, x, y, map, kind);
        check(kind == TB_OPAQUE ? "background row" : kind == TB_CLEAR ? "background row, clear 0" : "background row, blended", k);
    }
}

// filterScreenHue and filterScreenValue, as backgrnd.c has them
static void filterScreenHue(SDL_Surface *screen, Uint8 hue)
{
	Uint8 *screen_pixels = screen->pixels;
	int screen_pitch = screen->pitch;
	hue = hue << 4;
	Swar8 hues = SWAR8(hue);
	size_t i = 24;
	for (int y = 184; y > 0; --y)
	{
		for (int x = 264; x > 0; x -= sizeof(Swar8))
		{
			const Swar8 valuesMask = SWAR8(0x0F);
			Swar8 values = swar8Read(&screen_pixels[i]) & valuesMask;
			swar8Write(&screen_pixels[i], hues | values);
			i += sizeof(Swar8);
		}
		i += screen_pitch - 264;
	}
}
static void filterScreenValue(SDL_Surface *screen, Sint8 brightness)
{
	Uint8 *screen_pixels = screen->pixels;
	int screen_pitch = screen->pitch;
	if (brightness == 0)
		return;
	size_t i = 24;
	if (brightness <= -0xF || brightness > 0x1E)
	{
		for (int y = 184; y > 0; --y)
		{
			for (int x = 264; x > 0; x -= sizeof(Swar8))
			{
				Swar8 pixels = swar8Read(&screen_pixels[i]);
				const Swar8 huesMask = SWAR8(0xF0);
				Swar8 hues = pixels & huesMask;
				swar8Write(&screen_pixels[i], hues);
				i += sizeof(Swar8);
			}
			i += screen_pitch - 264;
		}
	}
	else if (brightness <= 0)
	{
		Swar8 addend = SWAR8((Uint8)(brightness + 0x10));
		for (int y = 184; y > 0; --y)
		{
			for (int x = 264; x > 0; x -= sizeof(Swar8))
			{
				Swar8 pixels = swar8Read(&screen_pixels[i]);
				const Swar8 huesMask = SWAR8(0xF0);
				Swar8 hues = pixels & huesMask;
				const Swar8 valuesMask = SWAR8(0x0F);
				Swar8 values = pixels & valuesMask;
				Swar8 sum = values + addend;
				Swar8 bit = sum & SWAR8(0x10);
				Swar8 mask = bit - (bit >> 4);
				values = sum & mask;
				swar8Write(&screen_pixels[i], hues | values);
				i += sizeof(Swar8);
			}
			i += screen_pitch - 264;
		}
	}
	else if (brightness <= 0xF)
	{
		Swar8 addend = SWAR8((Uint8)brightness);
		for (int y = 184; y > 0; --y)
		{
			for (int x = 264; x > 0; x -= sizeof(Swar8))
			{
				Swar8 pixels = swar8Read(&screen_pixels[i]);
				const Swar8 huesMask = SWAR8(0xF0);
				Swar8 hues = pixels & huesMask;
				const Swar8 valuesMask = SWAR8(0x0F);
				Swar8 values = pixels & valuesMask;
				Swar8 sum = values + addend;
				Swar8 bit = sum & SWAR8(0x10);
				Swar8 mask = bit - (bit >> 4);
				values = (sum & SWAR8(0x0F)) | mask;
				swar8Write(&screen_pixels[i], hues | values);
				i += sizeof(Swar8);
			}
			i += screen_pitch - 264;
		}
	}
	else
	{
		Swar8 addend = SWAR8((Uint8)(brightness - 0xF));
		for (int y = 184; y > 0; --y)
		{
			for (int x = 264; x > 0; x -= sizeof(Swar8))
			{
				Swar8 pixels = swar8Read(&screen_pixels[i]);
				const Swar8 huesMask = SWAR8(0xF0);
				Swar8 hues = pixels & huesMask;
				const Swar8 valuesMask = SWAR8(0x0F);
				Swar8 values = pixels & valuesMask;
				Swar8 sum = values + addend;
				Swar8 bit = sum & SWAR8(0x10);
				Swar8 mask = bit - (bit >> 4);
				values = SWAR8(0x0F) & ~mask;
				swar8Write(&screen_pixels[i], hues | values);
				i += sizeof(Swar8);
			}
			i += screen_pitch - 264;
		}
	}
}
void test_filter() {
    int k, b;
    SDL_Surface s;
    memset(&s, 0, sizeof s);
    s.pitch = W; s.w = W; s.h = H; s.pixels = bufc;
    for (k = 0; k < 120; k++) {
        clear();
        if (k & 1) {
            b = k / 2 - 30;    // -30 .. 29, and a few beyond
            if (k > 110) b = rnd(256) - 128;
            filterScreenValue(&s, (Sint8)b);
            tyr_filter(bufa, W, 1, (Sint8)b);
            check("filterScreenValue", k);
        } else {
            b = rnd(256);
            filterScreenHue(&s, (Uint8)b);
            tyr_filter(bufa, W, 0, (Uint8)b);
            check("filterScreenHue", k);
        }
    }
}

// tyrian-sum.c's checksum
Uint32 pal[256];
void test_sum() {
    int k, i, y;
    Uint32 h, c, a;
    Uint8 *p;
    for (k = 0; k < 30; k++) {
        fill(bufc, BUFN);
        for (i = 0; i < 256; i++) pal[i] = ((Uint32)rnd(1 << 16) << 16) | rnd(1 << 16);
        h = (Uint32)rnd(1 << 30) * 4;
        c = h;
        for (i = 0; i < 256; i++) c = c * 31 + pal[i];
        for (y = 0; y < H; y++) {
            p = bufc + y * W;
            for (i = 0; i < W; i += 4)
                c = c * 31 + (Uint32) (p[i] | (p[i + 1] << 8) | (p[i + 2] << 16) | ((Uint32) p[i + 3] << 24));
        }
        a = (Uint32)tyr_sum(h, pal, bufc, W, H, W);
        cases++;
        if (a != c) { fails++; if (fails < 10) printf("accel: sum, case %d, %08x, not %08x\n", k, (unsigned)a, (unsigned)c); }
    }
}

int main() {
    int i;
    if (TYR_ACCEL() <= 0) { printf("accel: Tyrian's are not here\n"); return 0; }
    bufc = malloc(BUFN); bufa = malloc(BUFN); base = malloc(BUFN);
    spr = malloc(8192); tiles = malloc(672 * 16);
    fill(base, BUFN); fill(tiles, 672 * 16);
    for (i = 0; i < 672 * 16; i++) if (rnd(3) == 0) tiles[i] = 0;
    test_sprite(); test_sprite2(); test_bgrow(); test_filter(); test_sum();
    if (fails) printf("accel: %d of %d cases differ\n", fails, cases);
    else printf("accel: Tyrian's, %d cases, each as its loop has it\n", cases);
    return fails != 0;
}
