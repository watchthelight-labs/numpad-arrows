#!/bin/sh
# <wtl> project: numpad-arrows | file: build.sh | author: watchthelight </wtl>
# Builds out/numpad-arrows-tap (universal x86_64 + arm64, ad-hoc signed). Needs the Xcode Command Line Tools.
set -eu
cd "$(dirname "$0")"
mkdir -p out
clang -O2 -Wall -Wextra -Wno-unused-parameter -mmacosx-version-min=11.0 \
    -arch x86_64 -arch arm64 \
    -framework ApplicationServices \
    -o out/numpad-arrows-tap src/numpad-arrows-tap.c
codesign -f -s - -i org.watchthelight.numpad-arrows out/numpad-arrows-tap
echo "built out/numpad-arrows-tap"
