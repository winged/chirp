#!/bin/sh

set -e

if [ "$CC" = "clang" ]; then
    ICLANG=clang
fi
pacman -Syu --noconfirm 2> /dev/null
pacman -S --noconfirm \
    sudo \
    base-devel \
    python-pip \
    python-sphinx \
    python-sphinx_rtd_theme \
    graphviz \
    cppcheck \
    openssl \
    libuv \
    valgrind \
    $ICLANG 2> /dev/null
if [ "$TESTSHELL" = "True" ]; then
    pacman -S --noconfirm gdb 2> /dev/null
fi
pip3 install pytest hypothesis u-msgpack-python
