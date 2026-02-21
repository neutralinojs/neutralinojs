#include <stdint.h>
#include <string>
#include "helpers.h"
#include "errors.h"

#if defined(__linux__)
#include <sys/sysinfo.h>
#include <gdk/gdk.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <cstdlib>

#elif defined(__FreeBSD__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <cstdlib>

#elif defined(__APPLE__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <CoreGraphics/CGEvent.h>

#elif defined(_WIN32)
#define _WINSOCKAPI_
#include <windows.h>
#endif


#include <infoware/system.hpp>
#include <infoware/cpu.hpp>
#include "api/computer/computer.h"
#include "api/window/window.h"
#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace computer {

string getArch() {
    iware::cpu::architecture_t architecture = iware::cpu::architecture();
    switch(architecture) {
        case iware::cpu::architecture_t::x64:
            return "x64";
        case iware::cpu::architecture_t::arm:
            return "arm";
        case iware::cpu::architecture_t::itanium:
            return "itanium";
        case iware::cpu::architecture_t::x86:
            return "ia32";
        default:
            return "unknown";
    }
}


pair<int, int> getMousePosition() {
    json output;
    int x, y;

    #if defined(__linux__)
    GdkDisplay* display = gdk_display_get_default();
    GdkSeat* seat = gdk_display_get_default_seat(display);
    GdkDevice* device = gdk_seat_get_pointer(seat);
    gdk_device_get_position(device, nullptr, &x, &y);

    #elif defined(_WIN32)
    POINT pos;
    GetCursorPos(&pos);
    x = pos.x;
    y = pos.y;

    #elif defined(__APPLE__)
    CGEventRef event = CGEventCreate(nullptr);
    CGPoint pos = CGEventGetLocation(event);
    x = pos.x;
    y = pos.y;
    CFRelease(event);
    #endif

    return make_pair(x, y);
}

static bool isWayland() {
#if defined(__linux__) || defined(__FreeBSD__)
    return getenv("WAYLAND_DISPLAY") != nullptr;
#else
    return false;
#endif
}

bool setCursorPosition(int x, int y, CoordSpace space) {
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
    else pt = CGPointMake(x, y);

    CGWarpMouseCursorPosition(pt);
    CGAssociateMouseAndMouseCursorPosition(true);
    return true;

#elif defined(__linux__) || defined(__FreeBSD__)
    if(isWayland()) return false;

    Display *d = XOpenDisplay(nullptr);
    if(!d) return false;

    XWarpPointer(d, None, DefaultRootWindow(d), 0, 0, 0, 0, x, y);
    XFlush(d);
    XCloseDisplay(d);
    return true;
#else
    return false;
#endif
}

bool setCursorGrab(bool enabled) {
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

bool sendKey(const string &key,
             const vector<string> &mods,
             const string &state) {
#if defined(_WIN32)
    if(key.size() != 1) return false;

    bool down = state != "up";
    WORD vk = VkKeyScanA(key[0]) & 0xFF;

    INPUT in{};
    in.type = INPUT_KEYBOARD;
    in.ki.wVk = vk;
    if(!down) in.ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(1, &in, sizeof(INPUT));

    if(state == "tap") {
        in.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &in, sizeof(INPUT));
    }
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

InputCapabilities getInputCapabilities() {
    InputCapabilities caps;
#if defined(_WIN32) || defined(__APPLE__)
    caps = {true, true, true};
#elif defined(__linux__) || defined(__FreeBSD__)
    if(isWayland())
        caps = {false, true, false};
    else
        caps = {true, true, true};
#endif
    return caps;
}

namespace controllers {

string __getKernelVariant(const iware::system::kernel_t &variant) {
	switch(variant) {
		case iware::system::kernel_t::windows_nt:
			return "Windows NT";
		case iware::system::kernel_t::linux:
			return "Linux";
		case iware::system::kernel_t::darwin:
			return "Darwin";
		default:
			return "Unknown";
	}
}

json getMemoryInfo(const json &input) {
    json output;
    const auto memory = iware::system::memory();

    output["returnValue"]["physical"] = {
        { "total", memory.physical_total },
        { "available", memory.physical_available }
    };

    output["returnValue"]["virtual"] = {
        { "total", memory.virtual_total },
        { "available", memory.virtual_available }
    };

    output["success"] = true;
    return output;
}

json getArch(const json &input) {
    json output;
    output["returnValue"] = computer::getArch();
    output["success"] = true;
    return output;
}

json getKernelInfo(const json &input) {
    json output;
    const auto kernelInfo = iware::system::kernel_info();
    string version = to_string(kernelInfo.major) + "." + to_string(kernelInfo.minor) + "." +
                            to_string(kernelInfo.patch) + "-" + to_string(kernelInfo.build_number);

    output["returnValue"] = {
        { "variant", __getKernelVariant(kernelInfo.variant) },
        { "version", version }
    };
    output["success"] = true;
    return output;
}

json getOSInfo(const json &input) {
    json output;
    const auto osInfo = iware::system::OS_info();
    string version = to_string(osInfo.major) + "." + to_string(osInfo.minor) + "." +
                            to_string(osInfo.patch) + "-" + to_string(osInfo.build_number);

    output["returnValue"] = {
        { "name", osInfo.name },
        { "description", osInfo.full_name },
        { "version", version }
    };
    output["success"] = true;
    return output;
}

json getCPUInfo(const json &input) {
    json output;
    const auto quantities = iware::cpu::quantities();

    output["returnValue"] = {
        { "vendor", iware::cpu::vendor() },
        { "model", iware::cpu::model_name() },
        { "frequency", iware::cpu::frequency() },
        { "architecture", computer::getArch() },
        { "logicalThreads", quantities.logical },
        { "physicalCores", quantities.physical },
        { "physicalUnits", quantities.packages }
    };
    output["success"] = true;
    return output;
}

json getDisplays(const json &input) {
    json output;
    output["returnValue"] = json::array();
    const auto displays = iware::system::displays();

    unsigned int displayId = 0;
    for(const auto &display: displays) {
        json displayInfo = {
            { "id", displayId },
            { "resolution", {
                { "width", display.width },
                { "height", display.height }
            }},
            { "dpi", display.dpi },
            { "bpp", display.bpp },
            { "refreshRate", display.refresh_rate }
        };

        output["returnValue"].push_back(displayInfo);
        displayId++;
    }
    output["success"] = true;
    return output;
}

json getMousePosition(const json &input) {
    json output;
    auto pos = computer::getMousePosition();

    json posRes = {
        {"x", pos.first},
        {"y", pos.second}
    };
    output["returnValue"] = posRes;
    output["success"] = true;
    return output;
}
json setCursorPosition(const json &input) {
    json output;

    const auto missing = helpers::missingRequiredField(input, {"x", "y"});
    if(missing) {
        output["error"] = errors::makeMissingArgErrorPayload(missing.value());
        return output;
    }

    int x = input["x"].get<int>();
    int y = input["y"].get<int>();

    string spaceStr = "window";
    if(helpers::hasField(input, "space"))
        spaceStr = input["space"].get<string>();

    CoordSpace space =
        (spaceStr == "screen") ? CoordSpace::Screen : CoordSpace::Window;

    if(!computer::setCursorPosition(x, y, space)) {
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

    if(!computer::setCursorGrab(enabled)) {
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

    string key = input["key"].get<string>();
    vector<string> mods;
    string state = "tap";

    if(helpers::hasField(input, "modifiers"))
        mods = input["modifiers"].get<vector<string>>();

    if(helpers::hasField(input, "state"))
        state = input["state"].get<string>();

    if(!computer::sendKey(key, mods, state)) {
        output["error"] = errors::makeErrorPayload(errors::NE_RT_NATRTER);
        return output;
    }

    output["success"] = true;
    return output;
}

json getInputCapabilities(const json &) {
    json output;

    InputCapabilities caps = computer::getInputCapabilities();
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
} // namespace computer
