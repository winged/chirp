#!/bin/sh

SIZE="$(du -s ci/arch/.cache | sed 's/\t.*//')"
if [ "$SIZE" -gt "280000" ]; then
    echo Cleaning cache
    rm -rf ci/arch/.cache
    mkdir -p ci/arch/.cache
    touch ci/arch/.cache/.keep
fi
