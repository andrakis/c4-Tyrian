# games/tyrian — Tyrian on C4IX, C4DOS and the bare machine

Tyrian (Eclipse Software, 1995), the vertical shooter, as OpenTyrian has
it. It is compiled by c4lc and played on libjs's whole display, with its
music on libjs's sound card AdLib and its sound effects on the card's
sample channels. It is played from the keyboard; the mouse works in the
menus.

The game's data is not here. Tyrian 2.1 is freeware, but its `license.doc`
does not allow it to be passed on. `data/README.md` says where to put
the zip so the build puts the game on the disks; without it the game is
built but says so when it starts.

[docs/ports-feasibility.md](../../docs/ports-feasibility.md) (§5) says
what c4lc needed for it. Doom ([games/doom](../doom/README.md)) was the
port before it, and this one follows it.

## Where it runs

| where | how | platform |
|---|---|---|
| C4IX, the whole display | `tyrian`, or Go > Tyrian; the desktop gives way until it ends (`include/display.h`) | `video-c4.c`, `tyr-c4ix.c` |
| C4IX, a window | `tyrian --window` in a Terminal, whose window it plays in; also when a program that does not give way has the display. With `pin`, on a CPU of its own (docs/c4ix-smp.md) | `video-c4.c`, `tyr-c4ix.c` |
| C4DOS | `TYRIAN` at the `A>` prompt; the page's `?system=c4dos` | `video-c4.c`, `tyr-bare.c` |
| the bare machine | `tyrian.c4r` as the machine's image; the page's `?system=tyrian` | `video-c4.c`, `tyr-bare.c` |

On C4IX the game asks for a nice of -5 (docs/c4ix-design.md 4.4), so it
is served sooner and longer than the programs beside it. It is
`/usr/games/tyrian/tyrian.c4r`, with its data in
`data/` next to it. The data stays on the host until something opens it:
that directory is a `hostdir()` in `c4ix.vfs.txt`, which takes whatever
files the disk's `tyrian/` has (`vfsload.c`). On the bare machine and
under C4DOS the data is in `tyrian/data` on the games' disk
(`src/c4bb/images/games`). In both cases the place comes from
`tyr_base()`, as a self-contained OpenTyrian's would. `-t DIR` overrides
it.

## Where it comes from

- **The game:** [OpenTyrian](https://github.com/opentyrian/opentyrian),
  commit `ac5d0e7`, under the GNU GPL v2 or later (`COPYING`).
  `README.opentyrian.md` is its own README. Its units and headers are
  here, without its build files and platform directories.
- **The data:** Tyrian 2.1, `tyrian21.zip`, from the user's own copy
  (`data/README.md`). `tools/tyrdata.py` takes the files the game reads
  from it: the levels, shapes, sounds, music, palettes, pictures and
  demos. It leaves the DOS programs, the documents and the network and
  setup files behind.

## What is here

| file | what |
|---|---|
| OpenTyrian's units and headers | the game, with the changes below |
| `c4sdl.c`, `c4/SDL.h` | the slice of SDL 2 the game calls, over libjs's display, clock and input |
| `video-c4.c` | in place of `video.c` and `video_scale*.c`: the 320x200 8-bit screen and its palette sent as `GUI_BLIT8`, which the host scales to 640x480 (2.4x down, the 4:3 a VGA monitor showed) |
| `loudness-c4.c` | in place of `loudness.c` and `opl.c`: the music player, `lds_play.c`, writes the card's own OPL, its updates stamped with the card's stream time at Tyrian's 69.5 a second; sound effects go on the sample channels as 8-bit |
| `tyrian-sum.c` | what every platform shares: `--timedemo N`, each frame shown checksummed with its palette, on a virtual clock |
| `tyr-c4ix.c`, `tyr-bare.c` | whether the display has to be asked for (on C4IX the desktop has it), and where the data is |
| `tyr-host.c` | the platform on a host, built with gcc: no display and no sound, the reference for `make test-tyrian` |
| `c4/` | `SDL.h` and friends, and the `stdint.h`, `stdbool.h` and `inttypes.h` C4IX's libc lacks |
| `tools/tyrdata.py` | the data out of the zip, for `build-images.sh` |
| `data/` | where the zip goes; ignored by git but for its README |

## The changes to OpenTyrian

Each is under `#ifdef C4TYR`. Everything else is OpenTyrian's own code,
and the units it replaces are not built.

- **`--timedemo N`** plays demo N from the title screen at once and exits
  when it ends (`opentyr.c`, `tyrian2.c`, `demo.c`).
- **The OPL** is the sound card's, not OpenTyrian's emulator (`opl.h`).
- **Speed.** `Swar8`, the type OpenTyrian's blitters work eight pixels
  at a time in, is the 32-bit word (`swar.h`). c4lc's `#if` arithmetic
  is a word wide, so upstream's `UINTPTR_MAX >= UINT64_MAX` test cannot
  tell 32 bits from 64. A 64-bit `Swar8` is two words, and every
  operation on it is a helper call: the background blitters and the
  starfield took 94% of the time. With the change the game runs twelve
  times as fast. gcc `-m32` makes the same choice.

c4lc needed M12 for it ([docs/c4lc-design.md](../../docs/c4lc-design.md)
14.11). That covers declarations in `for`, structs returned by value,
compound literals, `&A` of an array typedef, and scalar initializers that
convert between integer and floating types. Before the last of those,
`int t = c * r;` stored the float's bits. OpenTyrian's
`JE_gammaCorrect_func` is that line, and the game's palette went white.

## Building and testing

- `make tyrian-objs` compiles each unit with c4lc into `.tyrian/`, and
  the two platform choices into `.tyrian/plat/`. `build-images.sh` links
  them two ways:
  - against C4IX's libc, as `/usr/games/tyrian/tyrian.c4r`;
  - whole with the bare libc (`make wolf3d-bare-objs`), as
    `images/games/tyrian.c4r`.

  Where the zip is, it also unpacks the data onto both disks.
- `make tyrian-host` is the gcc build, 32-bit as the c4lc build is.
- `make test-tyrian` plays demo 4 with `--timedemo 4 -s -x -j` four ways:
  the gcc build, then c4lc's on the bare machine, under C4DOS and inside
  C4IX. All four must print the same frames. Without the data it says it
  was skipped.
  - `-s`: no sound, because the gcc build has none, so both run the same
    code.
  - `-x`: not the Christmas edition, which the date would choose in
    December.
  - `-j`: no joystick.

  These checksums are for the zip of `data/README.md`:

  ```
  timedemo: demo 1: 6051 frames, checksum 89ee7639
  timedemo: demo 2: 5120 frames, checksum 5860cfb7
  timedemo: demo 3: 2316 frames, checksum 5e5f6bb1
  timedemo: demo 4: 1856 frames, checksum 389b26f7
  timedemo: demo 5: 3105 frames, checksum 932401e1
  ```

  To compare how fast the game runs between machines, add `--show`: the
  frames go to the display (or to the window, with `--window`) as well, so the
  time counts the drawing. Every timedemo prints a speed line before its
  checksum, by the machine's real clock from the first frame:

  ```
  speed: demo 4: 1856 frames in 14.205 s, 152.9 fps
  ```

  In the page that clock is the wall's. Under `cli.js`, unpaced, it is
  cycles over the MHz asked for, so there time the run from outside.

  The game times itself by the clock, so while timing the clock is
  virtual: each reading is a millisecond later, and a delay adds its wait
  (`tyrian-sum.c`). `--trace` prints each frame's checksum as it goes, and
  `--rows N` prints frame N row by row with its palette. Comparing those
  with the gcc build's is how a difference is found.
