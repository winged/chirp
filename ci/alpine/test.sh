#!/bin/sh

set -e

mkdir -p /tmp/build
cd /tmp/build
/outside/configure --dev --doc
if [ "$TESTSHELL" = "True" ]; then
    exec /bin/sh
else
    if [ "$DOC_FORMAT" = "True" ]; then
        make doc
        make clean
        /outside/configure --doc
        make check
        make clean
        make install DEST=out
        [ -f out/usr/local/lib/libchirp.so ]
        make uninstall DEST=out
        [ ! -f out/usr/local/lib/libchirp.so ]
    else
        make test
        mkdir -p out
        make dist
        cd /tmp/build/dist
        make check
        cd /tmp/build
        make clean
        /outside/configure
        make check
        make install PREFIX=/usr DEST=out
        [ -f out/usr/lib/libchirp.so ]
        make uninstall PREFIX=/usr DEST=out
        [ ! -f out/usr/lib/libchirp.so ]
    fi
fi
