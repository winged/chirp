#!/bin/sh

cd /etc/apk
ln -s /outside/ci/alpine/.cache cache
cat > /etc/apk/repositories <<EOF
http://dl-4.alpinelinux.org/alpine/edge/main
http://dl-4.alpinelinux.org/alpine/edge/community
http://dl-4.alpinelinux.org/alpine/edge/testing
EOF
apk update
apk upgrade
apk add \
    sudo \
    alpine-sdk \
    py-sphinx \
    py-sphinx_rtd_theme \
    graphviz \
    cppcheck \
    openssl-dev \
    libuv-dev \
    clang
