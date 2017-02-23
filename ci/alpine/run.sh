#!/bin/sh

/outside/ci/alpine/install.sh
sudo -u \#$HUID /outside/ci/alpine/test.sh
