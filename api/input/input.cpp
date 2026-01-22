#include "input.h"
#include "helpers.h"
#include "errors.h"
#include "api/window/window.h"

#include <vector>
#include <string>
#include <cctype>


#if defined(_WIN32)
  #include <windows.h>
#elif defined(__APPLE__)
  #include <CoreGraphics/CoreGraphics.h>
#elif defined(__linux__) || defined(__FreeBSD__)
  #include <X11/Xlib.h>
  #include <X11/extensions/XTest.h>
  #include <cstdlib>
#endif

using namespace std;

namespace input {

    static bool isWayland() {
    #if defined(__linux__) || defined(__FreeBSD__)
        return std::getenv("WAYLAND_DISPLAY") != nullptr;
    #else
    return false;
    #endif
    }

// ---------------- Native implementations ----------------

bool setCursorPositionNative(int x, int y, CoordSpace space) {
#if defined(_WIN32)
    POINT p{ x, y };

    if(space == CoordSpace::Window) {
        HWND hwnd = (HWND)window::windowHandle;
        ClientToScreen(hwnd, &p);
    }

    return SetCursorPos(p.x, p.y) == TRUE;

#elif defined(__APPLE__)
    CGPoint pt;

    if(space == CoordSpace::Window) {
        CGRect main = CGDisplayBounds(CGMainDisplayID());
        pt = CGPointMake(x, main.size.height - y);
    }
    else {
        pt = CGPointMake(x, y);
    }

    CGWarpMouseCursorPosition(pt);
    CGAssociateMouseAndMouseCursorPosition(true);
    return true;

#elif defined(__linux__) || defined(__FreeBSD__)
    if(isWayland()) return false;

    Display *d = XOpenDisplay(nullptr);
    if(!d) return false;

    int sx = x, sy = y;
    XWarpPointer(d, None, DefaultRootWindow(d), 0, 0, 0, 0, sx, sy);
    XFlush(d);
    XCloseDisplay(d);
    return true;
#else
    return false;
#endif
}

bool setCursorGrabNative(bool enabled) {
#if defined(_WIN32)
    HWND hwnd = (HWND)window::windowHandle;

    if(enabled) {
        RECT r;
        GetClientRect(hwnd, &r);
        POINT tl{r.left, r.top}, br{r.right, r.bottom};
        ClientToScreen(hwnd, &tl);
        ClientToScreen(hwnd, &br);
        RECT clip{tl.x, tl.y, br.x, br.y};
        return ClipCursor(&clip) == TRUE;
    }
    return ClipCursor(nullptr) == TRUE;

#elif defined(__APPLE__)
    if(enabled) CGDisplayHideCursor(kCGDirectMainDisplay);
    else CGDisplayShowCursor(kCGDirectMainDisplay);
    return true;

#elif defined(__linux__) || defined(__FreeBSD__)
    if(isWayland()) return false;

    Display *d = XOpenDisplay(nullptr);
    if(!d) return false;

    if(enabled) {
        XGrabPointer(d, DefaultRootWindow(d), True,
            ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
            GrabModeAsync, GrabModeAsync,
            DefaultRootWindow(d), None, CurrentTime);
    }
    else {
        XUngrabPointer(d, CurrentTime);
    }

    XFlush(d);
    XCloseDisplay(d);
    return true;
#else
    return false;
#endif
}

static void applyModifiersWin(const std::vector<std::string>& mods, bool down) {
#if defined(_WIN32)
    for(const auto& m : mods) {
        WORD vk = 0;
        if(m == "Ctrl") vk = VK_CONTROL;
        else if(m == "Shift") vk = VK_SHIFT;
        else if(m == "Alt") vk = VK_MENU;
        else if(m == "Meta") vk = VK_LWIN;

        if(vk) {
            INPUT i{};
            i.type = INPUT_KEYBOARD;
            i.ki.wVk = vk;
            if(!down) i.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &i, sizeof(INPUT));
        }
    }
#endif
}

bool sendKeyNative(const std::string &key,
                   const std::vector<std::string> &mods,
                   const std::string &state) {
#if defined(_WIN32)
    if(key.size() != 1) return false;

    bool down = state != "up";

    applyModifiersWin(mods, true);

    INPUT in{};
    in.type = INPUT_KEYBOARD;
    in.ki.wVk = VkKeyScanA(key[0]) & 0xFF;
    if(!down) in.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &in, sizeof(INPUT));

    if(state == "tap") {
        in.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &in, sizeof(INPUT));
    }

    applyModifiersWin(mods, false);
    return true;

#elif defined(__APPLE__)
    if(key.size() != 1) return false;

    CGKeyCode code = (CGKeyCode)toupper(key[0]);
    bool down = state != "up";

    CGEventRef ev = CGEventCreateKeyboardEvent(nullptr, code, down);
    CGEventPost(kCGHIDEventTap, ev);
    CFRelease(ev);

    if(state == "tap") {
        ev = CGEventCreateKeyboardEvent(nullptr, code, false);
        CGEventPost(kCGHIDEventTap, ev);
        CFRelease(ev);
    }
    return true;

#elif defined(__linux__) || defined(__FreeBSD__)
    if(isWayland()) return false;

    if(key.size() != 1) return false;
    Display *d = XOpenDisplay(nullptr);
    if(!d) return false;

    KeyCode kc = XKeysymToKeycode(d, XStringToKeysym(key.c_str()));
    bool down = state != "up";

    XTestFakeKeyEvent(d, kc, down, CurrentTime);
    if(state == "tap")
        XTestFakeKeyEvent(d, kc, False, CurrentTime);

    XFlush(d);
    XCloseDisplay(d);
    return true;
#else
    return false;
#endif
}

InputCapabilities getCapabilitiesNative() {
    InputCapabilities caps;

#if defined(_WIN32)
    caps = {true, true, true};
#elif defined(__APPLE__)
    caps = {true, true, true};
#elif defined(__linux__) || defined(__FreeBSD__)
    if(isWayland()) {
        caps = {false, true, false};
    }
    else {
        caps = {true, true, true};
    }
#endif

    return caps;
}

namespace controllers {

json setCursorPosition(const json &input) {
    json output;

    const auto missing = helpers::missingRequiredField(input, {"x", "y"});
    if(missing) {
        output["error"] = errors::makeMissingArgErrorPayload(missing.value());
        return output;
    }

    int x = input["x"].get<int>();
    int y = input["y"].get<int>();

    std::string spaceStr = "window";
    if(helpers::hasField(input, "space"))
        spaceStr = input["space"].get<std::string>();

    CoordSpace space =
        (spaceStr == "screen") ? CoordSpace::Screen : CoordSpace::Window;

    if(!setCursorPositionNative(x, y, space)) {
        output["error"] = errors::makeErrorPayload(errors::NE_RT_NATRTER);
        return output;
    }

    output["success"] = true;
    return output;
}

json setCursorGrab(const json &input) {
    json output;

    bool enabled = true;
    if(helpers::hasField(input, "enabled"))
        enabled = input["enabled"].get<bool>();

    if(!setCursorGrabNative(enabled)) {
        output["error"] = errors::makeErrorPayload(errors::NE_RT_NATRTER);
        return output;
    }

    output["success"] = true;
    return output;
}

json sendKey(const json &input) {
    json output;

    const auto missing = helpers::missingRequiredField(input, {"key"});
    if(missing) {
        output["error"] = errors::makeMissingArgErrorPayload(missing.value());
        return output;
    }

    std::string key = input["key"].get<std::string>();
    std::vector<std::string> mods;
    std::string state = "tap";

    if(helpers::hasField(input, "modifiers"))
        mods = input["modifiers"].get<std::vector<std::string>>();

    if(helpers::hasField(input, "state"))
        state = input["state"].get<std::string>();

    if(!sendKeyNative(key, mods, state)) {
        output["error"] = errors::makeErrorPayload(errors::NE_RT_NATRTER);
        return output;
    }

    output["success"] = true;
    return output;
}

json getCapabilities(const json &) {
    json output;

    InputCapabilities caps = getCapabilitiesNative();
    json ret = {
        {"warp", caps.warp},
        {"grab", caps.grab},
        {"syntheticKeys", caps.syntheticKeys}
    };

    output["returnValue"] = ret;
    output["success"] = true;
    return output;
}

} // namespace controllers
} // namespace input
