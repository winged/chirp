#!/bin/sh

set -e

/outside/ci/arch/install.sh
sudo -E -u \#$HUID /outside/ci/arch/test.sh
