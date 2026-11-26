#!/bin/bash

set -ex

VERSION=$1

apt-get update
apt-get install -y libc++-${VERSION}-dev libc++abi-${VERSION}-dev
apt-get install -y --no-install-recommends libunwind-${VERSION}-dev
