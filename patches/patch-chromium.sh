#!/bin/sh

PATCH_DIR=`pwd`/src/ozone/patches/
HACKING_BRANCH=master-ozone

echo "Ozone-Wayland: patching Chromium"
cd src/

git am $PATCH_DIR/00*
