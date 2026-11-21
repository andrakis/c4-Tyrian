# Tyrian's data

OpenTyrian is the engine; the game is Tyrian 2.1's data, which the
repository does not have. Tyrian 2.1 was made freeware in 2004, but its
`license.doc` does not allow copying or distributing it, so it can't be
committed here. This directory ignores everything but this file.

To play it on c4, put the zip, as distributed, here:

    data/tyrian21.zip

You can also point at it from anywhere with `TYRIAN_ZIP=/path/to/tyrian21.zip`.
Then run `make c4bb-images`. `build-images.sh` unpacks the files the
game reads (`tools/tyrdata.py`) onto the disks it builds:

- C4IX's disk, under `tyrian/`. C4IX shows them in
  `/usr/games/tyrian/data` and fetches each one the first time it's
  opened.
- The games disk, under `tyrian/data`, for the bare machine and C4DOS.

Those disks are never committed either (`src/c4bb/images/` is ignored).

Without the zip everything still builds. Tyrian is linked as usual, but
when started it says its data is missing, and `make test-tyrian` reports
that it was skipped. C4IX's Go > Games and grin's Action Games then leave
plain Tyrian out.

The page can also take the zip from whoever plays it: its Uploads take
`tyrian21.zip` apart into `/home/tyrian` (c4's `libjs/uploads.js`), and
`tyrian --home` reads the data there (`/home/tyrian`, then `/home`; under
C4DOS `B:\TYRIAN`, then `B:\`; on the bare machine `tyrian/`, then the
top). That is how c4's GitHub Pages site offers Tyrian: it carries the
engine and none of the data (c4's `libjs/site.sh` refuses any of it).
The zip is the one OpenTyrian's README points to,
<https://camanis.net/tyrian/tyrian21.zip>.

`make test-tyrian`'s checksums are for the zip this port was made with:
`tyrian21.zip`, 4,754,048 bytes, holding a `tyrian21/` folder.
