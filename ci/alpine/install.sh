#!/bin/sh

set -e

cd /etc/apk
ln -s /outside/ci/alpine/.cache cache
cat > /etc/apk/repositories <<EOF
http://dl-cdn.alpinelinux.org/alpine/v3.6/main
http://dl-cdn.alpinelinux.org/alpine/v3.6/community
@edge http://dl-4.alpinelinux.org/alpine/edge/main
@comm http://dl-4.alpinelinux.org/alpine/edge/community
@test http://dl-4.alpinelinux.org/alpine/edge/testing
EOF

if [ "$TLS" = "openssl" ]; then
    export ITLS=openssl-dev
else
    export ITLS=libressl-dev
fi
apk update
apk upgrade
apk add \
    sudo \
    alpine-sdk \
    py-sphinx \
    py-sphinx_rtd_theme \
    graphviz \
    cppcheck \
    libuv-dev \
    abi-compliance-checker@edge \
    valgrind \
    clang \
    $ITLS
if [ "$TESTSHELL" = "True" ]; then
    apk add gdb
fi
