#!/bin/sh

set -e

SCRIPT="$(pwd -P)/$0"
BASE="${SCRIPT%/*}"
echo "Running arch docker test at $BASE"
cd "$BASE"

if [ -z "$CC" ]; then
    echo Set CC=gcc since CC is undefined
    export CC=gcc
fi

sudo docker run \
    -e "HUID=$(id -u)" \
    -e "CC=$CC" \
    -v "$(pwd -P)/..":/outside \
    --rm \
    base/archlinux:latest \
    /outside/ci/arch/run.sh
