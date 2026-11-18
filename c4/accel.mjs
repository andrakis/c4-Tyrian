// accel.mjs -- OpenTyrian's accelerators for c4's libjs (its
// libjs/accel.js, docs/accel.md): the sprites' blits in each of their
// kinds, the background's rows of tiles, the screen's hue and brightness
// filters and the timedemo's checksum, run by the host when the machine
// has them and Setup > Graphics' Accelerated is on. c4/c4accel.h is the
// game's side, with each operation's number (TA_*); sprite.c, backgrnd.c
// and tyrian-sum.c call them, and keep their own C for every other
// machine. Each does exactly what that C does: the same pixels in the same
// order, 32-bit arithmetic wrapping.
export default function tyrian({ reader, writer, reader32, probe, span, args }) {
  // The kinds of pixel, from the sprite's byte d and the screen's p; those
  // that read the screen are worked out first, every page they write made
  // sure of (probe), and only then written, so that a run again after a
  // fault under the MMU does not blend twice.
  const READS = new Set([1, 4, 5, 11, 12]);
  // a pixel's value for each kind: a and b are the call's own (hue and
  // value, black, filter)
  function pixel(kind, d, p, a, b) {
    switch (kind) {
    case 0: return d;                                                // blit_sprite
    case 1: return (d & 0xf0) | (((p & 0x0f) + (d & 0x0f)) >> 1);    // blit_sprite_blend
    case 2: return (a | ((d & 0x0f) + b)) & 0xff;                    // blit_sprite_hv_unsafe
    case 3: case 4: {                                                // blit_sprite_hv, _hv_blend
      let t = ((d & 0x0f) + b) & 0xff;
      if (t > 0xf) t = t >= 0x1f ? 0 : 0xf;
      return kind === 3 ? (a | t) & 0xff : (a | (((p & 0x0f) + t) >> 1)) & 0xff;
    }
    case 5: return b ? 0 : (p & 0xf0) | ((p & 0x0f) >> 1);          // blit_sprite_dark
    case 10: return d;                                               // blit_sprite2
    case 11: return (((d & 0x0f) + (p & 0x0f)) >> 1) | (d & 0xf0);   // blit_sprite2_blend
    case 12: return ((p & 0x0f) >> 1) + (p & 0xf0);                  // blit_sprite2_darken
    case 13: return (a | (d & 0x0f)) & 0xff;                         // blit_sprite2_filter
    }
    return d;
  }
  // the writes of one operation: straight to the screen, or held until
  // their pages are made sure of where the kind reads the screen
  function sink(m, held) {
    const wr = writer(m);
    if (!held) return { put: wr, done() {} };
    const wa = [], wv = [];
    return {
      put(x, v) { wa.push(x); wv.push(v); },
      done() {
        let page = -1;
        for (const x of wa) if ((x >>> 12) !== page) { probe(m, x, 1, true); page = x >>> 12; }
        for (let i = 0; i < wa.length; i++) wr(wa[i], wv[i]);
      },
    };
  }

  // TA_SUM (h, pal, pixels, w, hgt, pitch): tyrian-sum.c's checksum of a
  // frame, the palette's 256 words first, then each row's words as they lie
  function sum(m) {
    let [h, pal, pix, w, hgt, pitch] = args(m, 6);
    const r32 = reader32(m), rb = reader(m);
    for (let i = 0; i < 256; i++) h = (Math.imul(h, 31) + r32((pal + 4 * i) | 0)) | 0;
    for (let y = 0; y < hgt; y++) {
      const row = (pix + Math.imul(y, pitch)) | 0;
      const v = (row & 3) === 0 ? span(m, row, w + 3 & ~3, false) : null;
      for (let i = 0; i < w; i += 4) {
        const a = (row + i) | 0;
        const word = v ? (v[i] | (v[i + 1] << 8) | (v[i + 2] << 16) | (v[i + 3] << 24))
                       : (rb(a) | (rb((a + 1) | 0) << 8) | (rb((a + 2) | 0) << 16) | (rb((a + 3) | 0) << 24));
        h = (Math.imul(h, 31) + word) | 0;
      }
    }
    return h;
  }

  // TA_SPRITE (data, size, width, pixels, ll, ul, pitch, kind, a, b): the
  // sprite tables' blits, blit_sprite and its kinds 0 to 5: 255 n skips n,
  // 254 ends the row, 253 skips one, any other byte is a pixel; above ul it
  // stops, below ll it draws nothing
  function sprite(m) {
    let [data, size, width, pixels, ll, ul, pitch, kind, a, b] = args(m, 10);
    const rd = reader(m), rp = reader(m), out = sink(m, READS.has(kind));
    const end = (data + size) | 0;
    let xo = 0;
    for (; data < end; data = (data + 1) | 0) {
      const d = rd(data);
      if (d === 255) {
        data = (data + 1) | 0;
        const n = rd(data);
        pixels = (pixels + n) | 0; xo = (xo + n) >>> 0;
      } else if (d === 254) {
        pixels = (pixels + width - xo) | 0; xo = width;
      } else if (d === 253) {
        pixels = (pixels + 1) | 0; xo = (xo + 1) >>> 0;
      } else {
        if (pixels >= ul) break;
        if (pixels >= ll) out.put(pixels, pixel(kind, d, READS.has(kind) ? rp(pixels) : 0, a, b));
        pixels = (pixels + 1) | 0; xo = (xo + 1) >>> 0;
      }
      if (xo >= (width >>> 0)) { pixels = (pixels + pitch - xo) | 0; xo = 0; }
    }
    out.done();
    return 0;
  }

  // TA_SPRITE2 (data, pixels, ll, ul, vpitch, kind, a): the compressed
  // sprites' blits that do not clip at the sides, blit_sprite2 and its
  // kinds 10 to 13: a byte's low nibble skips, its high one draws that many
  // of the bytes after it, 0 of them ends the row (vpitch - 12 on, the
  // screen's pitch as the C has it), 0x0f ends the sprite
  function sprite2(m) {
    let [data, pixels, ll, ul, vpitch, kind, a] = args(m, 7);
    const rd = reader(m), rp = reader(m), out = sink(m, READS.has(kind));
    for (let c; (c = rd(data)) !== 0x0f; data = (data + 1) | 0) {
      pixels = (pixels + (c & 0x0f)) | 0;
      let count = c >> 4;
      if (count === 0) { pixels = (pixels + vpitch - 12) | 0; continue; }
      let stop = false;
      while (count--) {
        data = (data + 1) | 0;
        if (pixels >= ul) { stop = true; break; }
        if (pixels >= ll) out.put(pixels, pixel(kind, rd(data), READS.has(kind) ? rp(pixels) : 0, a, 0));
        pixels = (pixels + 1) | 0;
      }
      if (stop) break;
    }
    out.done();
    return 0;
  }

  // TA_SPRITE2C (data, x, y, pixels, pitch, h, kind, a): blit_sprite2_clip
  // (kind 10) and blit_sprite2_filter_clip (13), clipped on every side
  function sprite2c(m) {
    let [data, x, y, pix, pitch, h, kind, a] = args(m, 8);
    const rd = reader(m), wr = writer(m);
    for (let c; (c = rd(data)) !== 0x0f; data = (data + 1) | 0) {
      if (y >= h) break;
      let fill = c >> 4;
      x = (x + (c & 0x0f)) | 0;
      if (fill === 0) { y = (y + 1) | 0; x = (x - 12) | 0; }
      else if (y >= 0) {
        const row = (pix + Math.imul(y, pitch)) | 0;
        do {
          data = (data + 1) | 0;
          if (x >= 0 && x < pitch) wr((row + x) | 0, pixel(kind, rd(data), 0, a, 0));
          x = (x + 1) | 0;
        } while (--fill);
      } else {
        data = (data + fill) | 0;
        x = (x + fill) | 0;
      }
    }
    return 0;
  }

  // TA_BGROW (dst, pitch, x, y, map, kind): a row of 12 background tiles,
  // each 24x28, from map[0..11] (null: none), the rows above the screen's
  // top and below 184 left out: blitBackground1Row (kind 0, opaque),
  // blitBackgroundRow (1, 0 is clear) and blitBackgroundRowBlend (2)
  function bgrow(m) {
    let [dst, pitch, x, y, map, kind] = args(m, 6);
    const r32 = reader32(m), rs = reader(m), rp = reader(m), out = sink(m, kind === 2);
    let ty = 0, th = 28;
    if (y < 0) { ty = -y; y = 0; } else if (y > 184 - th) th = 184 - y;
    let di = (Math.imul(y, pitch) + x) | 0;
    const tiles = [];
    for (let ti = 0; ti < 12; ti++) tiles.push(r32((map + 4 * ti) | 0));
    for (; ty < th; ty++) {
      for (let ti = 0; ti < 12; ti++) {
        let data = tiles[ti];
        if (data === 0 && kind !== 0) { di = (di + 24) | 0; continue; }
        data = (data + ty * 24) | 0;
        for (let tx = 0; tx < 24; tx++, data = (data + 1) | 0, di = (di + 1) | 0) {
          const s = rs(data), at = (dst + di) | 0;
          if (kind === 0) out.put(at, s);
          else if (s !== 0) out.put(at, kind === 1 ? s : (s & 0xf0) | (((s & 0x0f) + (rp(at) & 0x0f)) >> 1));
        }
      }
      di = (di + pitch - 12 * 24) | 0;
    }
    out.done();
    return 0;
  }

  // TA_FILTER (pixels, pitch, kind, a): the play area (264x184 from x 24)
  // through filterScreenHue (kind 0, a the hue) or filterScreenValue (1, a
  // the brightness: a value of 0x0 to 0xF kept, 0x10 to 0x1E 0xF, else 0x0)
  function filter(m) {
    const [pix, pitch, kind, a] = args(m, 4);
    if (kind === 1 && a === 0) return 0;
    const rp = reader(m), out = sink(m, true), hue = (a << 4) & 0xff;
    let i = 24;
    for (let y = 184; y > 0; y--) {
      for (let x = 0; x < 264; x++, i++) {
        const at = (pix + i) | 0, p = rp(at);
        if (kind === 0) out.put(at, hue | (p & 0x0f));
        else {
          const v = (p & 0x0f) + a;
          out.put(at, (p & 0xf0) | (v >= 0 && v <= 0xf ? v : v >= 0x10 && v <= 0x1e ? 0xf : 0));
        }
      }
      i += pitch - 264;
    }
    out.done();
    return 0;
  }

  return [sum, sprite, sprite2, sprite2c, bgrow, filter];
}
