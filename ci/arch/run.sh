#!/bin/sh

set -e

export CI_DISTRO=arch
/outside/ci/arch/install.sh
sudo -E -u \#$HUID /outside/ci/arch/test.sh
