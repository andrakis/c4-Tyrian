#!/usr/bin/env python3
# tyrdata.py ZIP DEST -- the files OpenTyrian reads, out of Tyrian 2.1's
# zip (tyrian21.zip, as distributed: everything in a tyrian21/ folder)
# into DEST, flat. The DOS programs, their documents and the network and
# setup files are left in the zip. build-images.sh runs it only where the
# zip is (data/README.md): Tyrian's licence does not let
# it be passed on, so nothing here is in the repository.
import os, sys, zipfile

SKIP = ('.exe', '.doc', '.ovl', '.ico', '.diz', '.txt', '.int', '.box',
        '.ini', '.tfp', '.awe', '.bin')

def main():
    if len(sys.argv) != 3:
        sys.exit('usage: tyrdata.py ZIP DEST')
    z = zipfile.ZipFile(sys.argv[1])
    dest = sys.argv[2]
    os.makedirs(dest, exist_ok=True)
    n = 0
    for i in z.infolist():
        name = os.path.basename(i.filename).lower()
        if not name or name.endswith(SKIP) or name.startswith('net'):
            continue
        with open(os.path.join(dest, name), 'wb') as f:
            f.write(z.read(i))
        n += 1
    if n == 0 or not os.path.exists(os.path.join(dest, 'tyrian1.lvl')):
        sys.exit('tyrdata.py: %s is not Tyrian 2.1 (no tyrian1.lvl)' % sys.argv[1])
    print('tyrdata.py: %d files in %s' % (n, dest))

main()
