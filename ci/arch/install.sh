#!/bin/sh

set -e

if [ "$CC" = "clang" ]; then
    ICLANG=clang
fi
pacman -Syu --noconfirm
pacman -S --noconfirm \
    sudo \
    base-devel \
    python-sphinx \
    python-sphinx_rtd_theme \
    graphviz \
    cppcheck \
    openssl \
    libuv \
    valgrind \
    $ICLANG
