#!/bin/sh

mkdir -p /tmp/build && \
cd /tmp/build && \
/outside/configure \
    --dev --doc --ignore-coverage && \
make test && \
make doc
