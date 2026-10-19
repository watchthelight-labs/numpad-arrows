#!/bin/sh
# <wtl> project: numpad-arrows | file: uninstall.sh | author: watchthelight </wtl>
# Stops the LaunchAgent and removes it and the binary. The Accessibility entry stays until you remove it in
# System Settings > Privacy & Security > Accessibility (or run: tccutil reset Accessibility org.watchthelight.numpad-arrows).
set -eu
LABEL=org.watchthelight.numpad-arrows
PREFIX=${PREFIX:-$HOME/.local/bin}
launchctl bootout "gui/$(id -u)/$LABEL" 2>/dev/null || true
rm -f "$HOME/Library/LaunchAgents/$LABEL.plist" "$PREFIX/numpad-arrows-tap"
echo "removed $LABEL"
