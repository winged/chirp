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
    else
        make test
        mkdir -p out
        make install DEST=out
        [ -f out/usr/local/lib/libchirp.so ]
        make uninstall DEST=out
        [ ! -f out/usr/local/lib/libchirp.so ]
        make clean
        /outside/configure --doc
        make check
        /outside/configure --dev
        make dist
        cd dist
        make check
        make install PREFIX=/usr DEST=out
        [ -f out/usr/lib/libchirp.so ]
        make uninstall PREFIX=/usr DEST=out
        [ ! -f out/usr/lib/libchirp.so ]
    fi
fi
