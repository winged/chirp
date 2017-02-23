#!/bin/sh

SCRIPT="$(pwd -P)/$0"
BASE="${SCRIPT%/*}"
echo Running alpine docker test at $BASE
cd "$BASE"
sudo docker build  -t chirp-alpine alpine
sudo docker run \
    -u $(id -u) \
    -e "CC=$CC" \
    -v "$(pwd -P)/..":/outside \
    chirp-alpine
