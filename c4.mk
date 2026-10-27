# c4.mk -- this game's part of c4's Makefile. c4 includes it from
# games/tyrian/c4.mk, its submodule, and runs it from c4's root, with c4's
# tools and variables (games/README.md in c4). Without it, c4 builds
# everything else, and test-tyrian says it was skipped.

# OpenTyrian (games/tyrian/README.md): its units with c4lc, the same
# objects for C4IX, the bare machine and C4DOS but for the platform's
# (tyr-c4ix.c, tyr-bare.c, in .tyrian/plat), linked by build-images.sh as
# Doom's are. video-c4.c and loudness-c4.c stand in for upstream's
# video*.c, loudness.c and opl.c, which are not built. The game's data is
# not in the repository (games/tyrian/data/README.md).
TYR        := games/tyrian
TYR_PLAT   := tyr-host tyr-c4ix tyr-bare
TYR_SKIP   := video video_scale video_scale_hqNx loudness opl
TYR_UNITS  := $(filter-out $(TYR_PLAT) $(TYR_SKIP),$(patsubst $(TYR)/%.c,%,$(wildcard $(TYR)/*.c)))
TYR_HDRS   := $(wildcard $(TYR)/*.h $(TYR)/c4/*.h) \
              $(wildcard src/c4ix/libc/include/*.h src/c4ix/libc/include/sys/*.h) \
              src/c4ix/include/display.h libjs/guest/gui.h libjs/guest/gui.c \
              libjs/guest/snd.h libjs/guest/snd.c
TYR_CC     := ./c4sp32 -c 400000 src/c4sp/lisp/c4lc.lisp -O $(IMG_G) -mfuse -fno-builtin -c -conforming \
              -D C4TYR -I $(TYR)/c4 -I src/c4ix/libc/include -I src/c4ix/include \
              -I libjs/guest -I $(TYR)
tyrian-objs: $(TYR_UNITS:%=.tyrian/%.c4o) .tyrian/plat/tyr-c4ix.c4o .tyrian/plat/tyr-bare.c4o
.tyrian/%.c4o: $(TYR)/%.c $(TYR_HDRS) $(C4LC_LISP) .img_g | c4sp32
	@mkdir -p .tyrian/plat
	$(TYR_CC) $< $@ > /dev/null
.tyrian/plat/%.c4o: $(TYR)/%.c $(TYR_HDRS) $(C4LC_LISP) .img_g | c4sp32
	@mkdir -p .tyrian/plat
	$(TYR_CC) $< $@ > /dev/null
PHONY += tyrian-objs

# ... and by gcc for the host, with no display and no sound (tyr-host.c),
# the reference test-tyrian holds the c4lc build to; 32-bit, as that is
TYR_HOST_SRCS := $(filter-out $(TYR_SKIP:%=$(TYR)/%.c) $(TYR)/tyr-c4ix.c $(TYR)/tyr-bare.c, \
                 $(wildcard $(TYR)/*.c))
tyrian-host: $(TYR_HOST_SRCS) $(TYR_HDRS)
	gcc -m32 -O2 -std=gnu99 -w -Wno-error=implicit-function-declaration -DC4TYR -I$(TYR)/c4 -I$(TYR) -Ilibjs/guest -o $@ $(TYR_HOST_SRCS) -lm

# The data, which may not be passed on: the user's own copy of
# Tyrian 2.1, data/tyrian21.zip or $TYRIAN_ZIP (data/README.md). test-tyrian is
# skipped without it.

# test-tyrian: OpenTyrian's demo 4 (--timedemo, tyrian-sum.c: every frame
# shown and its palette, on a virtual clock), built by gcc and by c4lc --
# on the bare machine, under C4DOS and inside C4IX -- must show the same
# frames. -x: not the Christmas edition, which the date would choose.
# Only where Tyrian's data is (games/tyrian/data/README.md); skipped,
# and saying so, where it is not.
TYR_DEMO := 4
test-tyrian: tyrian-host c4bb-images
	@if [ ! -f src/c4bb/images/games/tyrian/data/tyrian1.lvl ]; then \
	   echo "test-tyrian: skipped, no Tyrian data (games/tyrian/data/README.md)"; exit 0; fi; \
	 rm -rf .tyr_t && mkdir -p .tyr_t && \
	 ./tyrian-host -t src/c4bb/images/games/tyrian/data --timedemo $(TYR_DEMO) -s -x -j 2>/dev/null \
	   | grep '^timedemo' > .tyr_t/host.txt && cat .tyr_t/host.txt && \
	 ( cd src/c4bb/images/games && { timeout 2400 node ../../../../libjs/cli.js -m 64 -d . tyrian.c4r --timedemo $(TYR_DEMO) -s -x -j \
	     > ../../../../.tyr_t/bare.out 2>/dev/null & \
	   printf 'tyrian --timedemo $(TYR_DEMO) -s -x -j\nver\n' | timeout 2400 node ../../../../libjs/cli.js -m 64 -d . ../c4dos32.c4r \
	     > ../../../../.tyr_t/dos.out 2>/dev/null; wait; } ) && \
	 printf 'tyrian --timedemo $(TYR_DEMO) -s -x -j\nexit\n' | timeout 2400 node libjs/cli.js -m 64 -d src/c4bb/images/disk src/c4bb/images/c4ix32.c4r \
	   2>/dev/null > .tyr_t/c4ix.out && \
	 for w in bare dos c4ix; do grep '^timedemo' .tyr_t/$$w.out | cmp -s .tyr_t/host.txt - \
	   || { echo "test-tyrian: $$w's frames differ:"; tail -5 .tyr_t/$$w.out; exit 1; }; done && \
	 { grep -A1 '^timedemo' .tyr_t/dos.out | grep -Eq '^A(:\\)?>C4DOS version' \
	   || { echo "test-tyrian: C4DOS did not come back to its prompt:"; tail -5 .tyr_t/dos.out; exit 1; }; } && \
	 rm -rf .tyr_t && echo "test-tyrian: OK"
PHONY += test-tyrian

GAME_OBJS  += tyrian-objs
GAME_TESTS += test-tyrian
