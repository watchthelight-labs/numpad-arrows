# numpad-arrows

> **Heads up:** a good portion of this code is AI generated. I do understand how it works,
> and I'm happy to explain any of it. If something looks wrong or you have a question,
> open an issue or ping me on Discord (`watchthelight`).

A small macOS event tap that turns numpad 1, 5, 2 and 3 into the Left, Up, Down and Right arrow keys. It runs as a user LaunchAgent, needs Accessibility permission, and is about 60 lines of C with no driver and no kext.

I wrote it because the arrow keys on my Lenovo Legion 5 15IAX10 are a hardware fault, and the usual fix, `hidutil property --set '{"UserKeyMapping":...}'`, does nothing on that keyboard. It is the ITE 8258 USB keyboard, which macOS publishes with a vendor usage instead of the keyboard usage, so the HID keyboard filter that applies `UserKeyMapping` never attaches to it. An event tap sits after all of that, in the window server's event stream, so it works for any keyboard.

## When to use it

- Your arrow keys are broken or missing and you have a numpad.
- `hidutil` remapping is silently ignored for your keyboard (check below), and you don't want a virtual HID driver for one remap.

If `hidutil` does work for your keyboard, use that instead: it remaps below the window server and needs no permission.

To check whether `hidutil` sees your keyboard as a keyboard:

```
hidutil list --matching '{"PrimaryUsagePage":1,"PrimaryUsage":6}'
```

If your keyboard is not in that list, `UserKeyMapping` will not reach it.

## Mapping

```
        [5]            [5] Up
    [1] [2] [3]    [1] Left   [2] Down   [3] Right
```

The four keys form an inverted T, like real arrow keys. Every other key, the top-row digits included, is left alone. While the tap runs, keypad 1, 5, 2 and 3 no longer type digits; NumLock does not change that (see [docs/HOW-IT-WORKS.md](docs/HOW-IT-WORKS.md#numlock)).

## Install

Needs the Xcode Command Line Tools (`xcode-select --install`).

```
./build.sh       # out/numpad-arrows-tap, universal (Intel and Apple silicon), ad-hoc signed
./install.sh     # copies it to ~/.local/bin and loads the LaunchAgent; no sudo
```

`PREFIX=/some/dir ./install.sh` installs the binary somewhere else. `org.watchthelight.numpad-arrows.plist` in the repo has `__BIN__` where the binary path goes; `install.sh` fills it in and writes the result to `~/Library/LaunchAgents/`. If you install by hand, put the full path there yourself.

## Accessibility permission

The tap changes key events, so macOS wants Accessibility access for it. The first start asks for it. If no prompt appears, open System Settings > Privacy & Security > Accessibility, click +, pick the installed binary (`~/.local/bin/numpad-arrows-tap`; in the file dialog, Cmd+Shift+G and paste the path) and switch it on.

Until access is given the program exits with `no event tap`, and launchd starts it again every 10 seconds, so the keys start working shortly after you flip the switch. No restart or log out needed.

The binary is ad-hoc signed, so macOS ties the permission to that exact build. After a rebuild and reinstall, switch the entry off and on again (or remove it and add it back).

Check that it runs:

```
launchctl print gui/$(id -u)/org.watchthelight.numpad-arrows | grep -E 'state|last exit'
```

`state = running` means it is working. A repeating `last exit code = 1` means it still has no Accessibility access.

## Changing the keys

The mapping is the `kMap` table at the top of [src/numpad-arrows-tap.c](src/numpad-arrows-tap.c): pairs of macOS virtual keycodes, the key you press and the key it becomes. [docs/HOW-IT-WORKS.md](docs/HOW-IT-WORKS.md#keycodes) lists the keypad, arrow and navigation keycodes. For example, to use 8/4/5/6 instead:

```c
    { 0x5B, 0x7E },   // keypad 8 -> up arrow
    { 0x56, 0x7B },   // keypad 4 -> left arrow
    { 0x57, 0x7D },   // keypad 5 -> down arrow
    { 0x58, 0x7C },   // keypad 6 -> right arrow
```

Then `./build.sh && ./install.sh`, and switch the Accessibility entry off and on.

## Uninstall

```
./uninstall.sh
```

It stops the agent and removes the agent and the binary. The Accessibility entry stays in System Settings until you remove it there.

To pause it without uninstalling: `launchctl bootout gui/$(id -u)/org.watchthelight.numpad-arrows`. `./install.sh` brings it back.

## What it can't do

- Password fields and anything else that turns on Secure Keyboard Entry: macOS hides those keystrokes from every event tap, so the numpad types digits there.
- The login window and the FileVault unlock screen: the agent starts when you log in.
- It remaps those keys on every keyboard at once, not one device. The tap sees events after macOS has merged all keyboards.

## Status

In daily use on a Lenovo Legion 5 15IAX10 with macOS Tahoe 26.7.1 (25G241), since 2026-10-16. Built with Apple clang from the Command Line Tools. Only tested on that machine; reports from others are welcome.

## How it works

A session event tap (`CGEventTapCreate` at `kCGSessionEventTap`) gets every key down and key up, swaps the keycode when it is in the table, adds the numeric-pad and fn flags that real arrow keys carry, and passes the event on. If macOS disables the tap for being slow, it turns it back on. More detail, including why not `hidutil`: [docs/HOW-IT-WORKS.md](docs/HOW-IT-WORKS.md).

## Licence

ISC. See [LICENSE](LICENSE).
