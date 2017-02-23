#!/bin/sh

brew install \
    cppcheck \
    sphinx-doc \
    graphviz \
    openssl \
    libuv
cd build && \
../configure \
    --dev \
    --doc \
    --ignore-coverage && \
make test && \
make doc

