#!/bin/sh

set -e

mkdir -p /tmp/build
cd /tmp/build
/outside/configure
make
make check
