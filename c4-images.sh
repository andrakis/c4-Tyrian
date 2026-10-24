# c4-images.sh c4ix|bare -- OpenTyrian's part of c4's build-images.sh,
# which sources it from c4's root (games/README.md in c4)
GAMEDIR=games/tyrian
# Tyrian 2.1's data may not be passed on, so it is not in the repository:
# where the user's zip is (data/README.md), the files the game reads go in
# the disk's tyrian/, which C4IX shows as /usr/games/tyrian/data
# (c4ix.vfs.txt, hostdir); where it is not, the game says so when it starts.
TYRIAN_ZIP=${TYRIAN_ZIP:-$GAMEDIR/data/tyrian21.zip}
case "$1" in
c4ix)
    rm -rf $DISK/tyrian
    if ls .tyrian/*.c4o > /dev/null 2>&1 && [ -f $PB/lib/libc.c4s ]; then
        say "linking OpenTyrian for C4IX"
        ./c4rlink32 .tyrian/*.c4o .tyrian/plat/tyr-c4ix.c4o -l $PB/lib/libc.c4s -o $DISK/c4ix-tyrian.c4r > /dev/null
        if [ -f "$TYRIAN_ZIP" ]; then python3 -I $GAMEDIR/tools/tyrdata.py "$TYRIAN_ZIP" $DISK/tyrian > /dev/null; fi
    fi ;;
bare)
    if ls .tyrian/*.c4o > /dev/null 2>&1; then
        item tyrian
        ./c4rlink32 .tyrian/*.c4o .tyrian/plat/tyr-bare.c4o .libc-bare/libc-*.c4o -o $GAMESDISK/tyrian.c4r > /dev/null
        if [ -f "$TYRIAN_ZIP" ]; then python3 -I $GAMEDIR/tools/tyrdata.py "$TYRIAN_ZIP" $GAMESDISK/tyrian/data > /dev/null; fi
        GAMES_ON_DISK="$GAMES_ON_DISK TYRIAN"
    fi ;;
esac
