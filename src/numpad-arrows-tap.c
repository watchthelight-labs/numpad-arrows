// <wtl> project: numpad-arrows | file: src/numpad-arrows-tap.c | author: watchthelight </wtl>
// Num1/Num5/Num2/Num3 act as Left/Up/Down/Right, for keyboards whose arrow keys are broken. Some keyboards (the ITE
// 8258 USB keyboard in the Lenovo Legion 5 15IAX10, for one) are published by macOS with a vendor usage, so no
// keyboard filter attaches and hidutil UserKeyMapping on them is ignored. This session event tap rewrites the key
// events instead, for every keyboard. Needs Accessibility permission. Run by the LaunchAgent
// org.watchthelight.numpad-arrows.
#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>

// The mapping. Left column: the key you press, right column: the key macOS sees. Values are macOS virtual keycodes
// (kVK_* in <HIToolbox/Events.h>); docs/HOW-IT-WORKS.md lists the keypad and arrow ones. Rebuild after a change.
static const struct { int64_t from, to; } kMap[] = {
    { 0x53, 0x7B },   // keypad 1 -> left arrow
    { 0x57, 0x7E },   // keypad 5 -> up arrow
    { 0x54, 0x7D },   // keypad 2 -> down arrow
    { 0x55, 0x7C },   // keypad 3 -> right arrow
};

static CFMachPortRef gTap;

static CGEventRef cb(CGEventTapProxy proxy, CGEventType type, CGEventRef ev, void *ctx) {
    if (type == kCGEventTapDisabledByTimeout || type == kCGEventTapDisabledByUserInput) {
        CGEventTapEnable(gTap, true);   // macOS turns a slow tap off; turn it back on
        return ev;
    }
    if (type != kCGEventKeyDown && type != kCGEventKeyUp) return ev;
    int64_t kc = CGEventGetIntegerValueField(ev, kCGKeyboardEventKeycode);
    int64_t to = -1;
    for (size_t i = 0; i < sizeof kMap / sizeof kMap[0]; i++)
        if (kMap[i].from == kc) { to = kMap[i].to; break; }
    if (to < 0) return ev;
    CGEventSetIntegerValueField(ev, kCGKeyboardEventKeycode, to);
    // arrow keys carry the numeric-pad and fn flags on Apple keyboards; text fields rely on them
    CGEventSetFlags(ev, CGEventGetFlags(ev) | kCGEventFlagMaskNumericPad | kCGEventFlagMaskSecondaryFn);
    return ev;
}

int main(void) {
    // ask macOS to show the Accessibility prompt the first time (no-op once granted)
    const void *k[] = { kAXTrustedCheckOptionPrompt }, *v[] = { kCFBooleanTrue };
    CFDictionaryRef opt = CFDictionaryCreate(NULL, k, v, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    AXIsProcessTrustedWithOptions(opt);
    CFRelease(opt);
    CGEventMask mask = CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventKeyUp);
    gTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, mask, cb, NULL);
    if (!gTap) {
        fprintf(stderr, "numpad-arrows-tap: no event tap (grant Accessibility to this binary)\n");
        return 1;   // launchd restarts it; it works as soon as permission is given
    }
    CFRunLoopSourceRef src = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, gTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), src, kCFRunLoopCommonModes);
    CGEventTapEnable(gTap, true);
    CFRunLoopRun();
    return 0;
}
