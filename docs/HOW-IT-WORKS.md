# How it works

## Why an event tap and not hidutil

macOS has two places where a key can be remapped without a driver.

The first is `hidutil property --set '{"UserKeyMapping":[...]}'`. It sets a property on the HID event services, and the keyboard filter inside the HID event system applies it as each key comes off the device. That filter only attaches to services that look like a keyboard: primary usage page 1 (Generic Desktop), usage 6 (Keyboard). Many laptop keyboards are fine. The ITE 8258 keyboard in the Legion 5 15IAX10 is a USB device whose top-level collection is a vendor usage, so macOS publishes the service with that vendor usage (`hidutil list` shows it as usage page 65417, 0xFF89, usage 16). Keys still arrive, but no keyboard filter is attached to them, and `UserKeyMapping` is accepted and ignored. The same goes for the VoodooPS2 keyboard map, since that keyboard is not on PS/2 at all.

The second is a Quartz event tap. It sees events in the window server after the HID layer has turned them into key events for the session, whatever device they came from. So it works for this keyboard, at the cost of needing Accessibility permission and running as a process.

A third option is Karabiner-Elements, which grabs the device through its own virtual HID driver. That may work too; this tool exists so that one remap does not need a system extension.

## The tap

`main` asks for Accessibility access with `AXIsProcessTrustedWithOptions` (that shows the system prompt once), then creates the tap:

```c
CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault,
                 CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventKeyUp), cb, NULL);
```

- `kCGSessionEventTap`: events for the logged-in user's session, which is where a LaunchAgent lives.
- `kCGHeadInsertEventTap`: ahead of other taps, so other tools see the arrow key, not the digit.
- `kCGEventTapOptionDefault`: an active tap that may change events. This is what needs Accessibility; a listen-only tap would need Input Monitoring instead and could not change anything.

Without permission, `CGEventTapCreate` returns NULL. The program prints `no event tap` and exits 1, and the LaunchAgent (`KeepAlive`, `ThrottleInterval` 10) starts it again ten seconds later. Once access is given, the next start works.

For each key down and key up, the callback reads `kCGKeyboardEventKeycode`, looks it up in `kMap`, writes the new keycode back and returns the same event. Key repeat works because repeats are key downs too. Modifiers pass through, so Shift+keypad 1 is Shift+Left and selects text.

It also sets `kCGEventFlagMaskNumericPad` and `kCGEventFlagMaskSecondaryFn` on the rewritten event. Real arrow keys on Apple keyboards carry both, and some text views look at them to tell an arrow from other keys. The keypad keys already carry the numeric-pad flag, so in practice this adds fn.

macOS disables a tap that takes too long to answer (`kCGEventTapDisabledByTimeout`) or for user input reasons (`kCGEventTapDisabledByUserInput`). The callback gets that as an event type and turns the tap back on.

## Keycodes

macOS virtual keycodes (the `kVK_*` names in `<HIToolbox/Events.h>`). They are the same on every keyboard layout for these keys.

| Key | Keycode | | Key | Keycode |
|---|---|---|---|---|
| keypad 0 | 0x52 | | left arrow | 0x7B |
| keypad 1 | 0x53 | | right arrow | 0x7C |
| keypad 2 | 0x54 | | down arrow | 0x7D |
| keypad 3 | 0x55 | | up arrow | 0x7E |
| keypad 4 | 0x56 | | home | 0x73 |
| keypad 5 | 0x57 | | end | 0x77 |
| keypad 6 | 0x58 | | page up | 0x74 |
| keypad 7 | 0x59 | | page down | 0x79 |
| keypad 8 | 0x5B | | forward delete | 0x75 |
| keypad 9 | 0x5C | | return | 0x24 |
| keypad . | 0x41 | | escape | 0x35 |
| keypad enter | 0x4C | | tab | 0x30 |
| keypad + | 0x45 | | | |
| keypad - | 0x4E | | | |
| keypad * | 0x43 | | | |
| keypad / | 0x4B | | | |
| keypad = | 0x51 | | | |
| clear (NumLock on PC keyboards) | 0x47 | | | |

Note keypad 8 is 0x5B, not 0x5A. To find the code of any other key, run a listen-only tool such as Key Codes, or temporarily add an `fprintf(stderr, "%lld\n", kc);` to the callback and watch the output.

Keys outside the keypad, like F-keys, work in the table the same way, but the numeric-pad and fn flags are only right for arrow and navigation keys. Drop that `CGEventSetFlags` line if you map to letters or digits.

## NumLock

macOS has no NumLock state for the numeric keypad. A PC keyboard's NumLock key arrives as keypad Clear (0x47), and the keypad keeps sending its digit keycodes whatever the NumLock light shows. So the tap cannot follow NumLock: while it runs, keypad 1, 5, 2 and 3 are always arrows. Type those digits on the top row, or pause the agent with `launchctl bootout gui/$(id -u)/org.watchthelight.numpad-arrows`.

If you want a toggle, the Clear key is the obvious one to use: catch 0x47 in the callback, flip a flag, and skip the mapping while it is off. It is not in 0.1.0 because nobody has needed it yet.

## Limits

- Secure Keyboard Entry (password fields, Terminal's Secure Keyboard Entry option) hides keystrokes from all event taps, so the numpad types digits there.
- The agent runs in the user session, so nothing is remapped at the login window or FileVault unlock.
- The tap sees the merged event stream, so it cannot tell which keyboard a key came from. An external keyboard's numpad is remapped too.
