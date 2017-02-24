#!/bin/sh

SCRIPT="$(pwd -P)/$0"
BASE="${SCRIPT%/*}"
echo Running osx test at $BASE
cd "$BASE"
brew install \
    cppcheck \
    sphinx-doc \
    graphviz \
    openssl \
    libuv 2>/dev/null
cd ../build && \
../configure \
    --dev \
    --doc \
    --ignore-coverage && \
make test && \
make doc

