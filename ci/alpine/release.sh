#!/bin/sh

set -e

mkdir -p /tmp/build
cd /tmp/build
/outside/configure --doc
make
make check
make doc
