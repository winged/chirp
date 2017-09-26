#!/bin/sh

set -e

mkdir -p /tmp/build
cd /tmp/build
/outside/configure --dev --doc
if [ "$TESTSHELL" = "True" ]; then
    exec /bin/sh
else
    make test
    make doc
    mkdir -p out
    make install DEST=out
    [ -f out/usr/local/lib/libchirp.so ]
    make uninstall DEST=out
    [ ! -f out/usr/local/lib/libchirp.so ]
    make clean
fi
