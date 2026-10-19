#!/bin/sh
# <wtl> project: numpad-arrows | file: install.sh | author: watchthelight </wtl>
# Installs the tap for the current user: the binary to $PREFIX (default ~/.local/bin) and a LaunchAgent that keeps
# it running. No sudo. Run ./build.sh first.
set -eu
cd "$(dirname "$0")"
LABEL=org.watchthelight.numpad-arrows
PREFIX=${PREFIX:-$HOME/.local/bin}
BIN=$PREFIX/numpad-arrows-tap
AGENT=$HOME/Library/LaunchAgents/$LABEL.plist

[ -x out/numpad-arrows-tap ] || { echo "out/numpad-arrows-tap missing, run ./build.sh first" >&2; exit 1; }
mkdir -p "$PREFIX" "$HOME/Library/LaunchAgents"
launchctl bootout "gui/$(id -u)/$LABEL" 2>/dev/null || true
# copy beside and rename, so a running copy is never overwritten in place
cp out/numpad-arrows-tap "$BIN.new" && mv -f "$BIN.new" "$BIN"
sed "s|__BIN__|$BIN|" $LABEL.plist > "$AGENT"
launchctl bootstrap "gui/$(id -u)" "$AGENT"
echo "installed $BIN and $AGENT"
echo
echo "macOS now asks for Accessibility access for numpad-arrows-tap. If no prompt shows up, open"
echo "System Settings > Privacy & Security > Accessibility, add $BIN with the + button and switch it on."
echo "The agent retries every 10 s, so the keys start working shortly after access is given."
