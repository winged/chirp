#!/bin/sh

set -e

cd /var/cache/pacman
rm -d pkg
ln -s /outside/ci/arch/.cache pkg
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
