#!/bin/sh

set -e

/outside/ci/alpine/install.sh
sudo -E -u \#$HUID /outside/ci/alpine/test.sh
