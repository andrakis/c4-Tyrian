/* c4accel.h -- c4's port: OpenTyrian's accelerators, c4/accel.mjs, from
   the game's side (c4's libjs/guest/accel.h, docs/accel.md). On a machine
   that has them, with Setup > Graphics' Accelerated on, the host runs these
   in the game's own JavaScript, one instruction each; elsewhere, and in
   gcc's build, the C runs as before. tyr_accel is the first of the game's
   numbers (0 for none, -1 until asked: tyrian-sum.c); TYR_ACCEL() asks
   once. The numbers are the module's operations in its order. */
#ifndef C4ACCEL_H
#define C4ACCEL_H

enum { TA_SUM, TA_SPRITE, TA_SPRITE2, TA_SPRITE2C, TA_BGROW, TA_FILTER };
/* the kinds of sprite pixel (sprite.c): the sprite tables' blits, then the
   compressed sprites' */
enum { TS_PLAIN, TS_BLEND, TS_HV_UNSAFE, TS_HV, TS_HV_BLEND, TS_DARK,
       TS_2 = 10, TS_2_BLEND, TS_2_DARKEN, TS_2_FILTER };
/* the background's rows (backgrnd.c): opaque, 0 clear, blended */
enum { TB_OPAQUE, TB_CLEAR, TB_BLEND };

extern int tyr_accel;
int tyr_accel_base(void);
int tyr_accel_avail(void);      /* the machine has them */
void tyr_accel_set(int on);     /* the menu's switch */
extern int tyr_accel_on;        /* Accelerated, "accelerated" in the config */
#define TYR_ACCEL() (tyr_accel >= 0 ? tyr_accel : tyr_accel_base())

#ifdef __c4_inttypes__
#include "accel.h"
#define TA_OP(op) (tyr_accel + (op))
/* tyrian-sum.c's checksum of a frame and its palette, from h */
#define tyr_sum(h, pal, pixels, w, hgt, pitch) \
	__c4_opcode((h), (pal), (pixels), (w), (hgt), (pitch), TA_OP(TA_SUM))
/* sprite.c's blit_sprite and its kinds (TS_PLAIN to TS_DARK): a and b are
   hue (shifted) and value, or b black for TS_DARK */
#define tyr_sprite(data, size, width, pixels, ll, ul, pitch, kind, a, b) \
	__c4_opcode((data), (size), (width), (pixels), (ll), (ul), (pitch), (kind), (a), (b), TA_OP(TA_SPRITE))
/* blit_sprite2 and its kinds that do not clip at the sides (TS_2 to
   TS_2_FILTER, a the filter); vpitch is VGAScreen's, as the C steps rows */
#define tyr_sprite2(data, pixels, ll, ul, vpitch, kind, a) \
	__c4_opcode((data), (pixels), (ll), (ul), (vpitch), (kind), (a), TA_OP(TA_SPRITE2))
/* blit_sprite2_clip (TS_2) and blit_sprite2_filter_clip (TS_2_FILTER) */
#define tyr_sprite2c(data, x, y, pixels, pitch, h, kind, a) \
	__c4_opcode((data), (x), (y), (pixels), (pitch), (h), (kind), (a), TA_OP(TA_SPRITE2C))
/* backgrnd.c's rows of 12 tiles (TB_*) */
#define tyr_bgrow(dst, pitch, x, y, map, kind) \
	__c4_opcode((dst), (pitch), (x), (y), (map), (kind), TA_OP(TA_BGROW))
/* filterScreenHue (kind 0, a the hue) and filterScreenValue (1, a the
   brightness) */
#define tyr_filter(pixels, pitch, kind, a) \
	__c4_opcode((pixels), (pitch), (kind), (a), TA_OP(TA_FILTER))
#else
#define accel_find(name) 0
#define tyr_sum(h, pal, pixels, w, hgt, pitch) 0
#define tyr_sprite(data, size, width, pixels, ll, ul, pitch, kind, a, b) 0
#define tyr_sprite2(data, pixels, ll, ul, vpitch, kind, a) 0
#define tyr_sprite2c(data, x, y, pixels, pitch, h, kind, a) 0
#define tyr_bgrow(dst, pitch, x, y, map, kind) 0
#define tyr_filter(pixels, pitch, kind, a) 0
#endif

#endif
