#!/bin/sh

set -e

export CI_DISTRO=alpine
/outside/ci/alpine/install.sh
if [ "$MODE" == "release" ]; then
    sudo -E -u \#$HUID /outside/ci/alpine/release.sh
else
    sudo -E -u \#$HUID /outside/ci/alpine/test.sh
fi
