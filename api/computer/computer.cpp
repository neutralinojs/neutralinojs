#include <stdint.h>
#include <string>
#include "helpers.h"
#include "errors.h"

#if defined(__linux__)
#include <sys/sysinfo.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <cstdlib>

#elif defined(__FreeBSD__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

#elif defined(__APPLE__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <CoreGraphics/CGEvent.h>
#include <CoreGraphics/CGEventSource.h>

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

bool setMousePosition(int x, int y) {
    #if defined(_WIN32)
    POINT p { x, y };
    return SetCursorPos(p.x, p.y);

    #elif defined(__APPLE__)
    CGPoint p = CGPointMake(x, y);
    CGWarpMouseCursorPosition(p);
    return true;

    #elif defined(__linux__) || defined(__FreeBSD__)
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

bool setMouseGrabbing(bool grabbing = true) {
    #if defined(_WIN32)
    HWND hwnd = window::getHandle();

    if(grabbing) {
        RECT r;
        GetClientRect(hwnd, &r);
        POINT tl {r.left, r.top}, br {r.right, r.bottom};
        ClientToScreen(hwnd, &tl);
        ClientToScreen(hwnd, &br);
        RECT clip {tl.x, tl.y, br.x, br.y};
        return ClipCursor(&clip);
    }
    return ClipCursor(nullptr);

    #elif defined(__APPLE__)
    if(grabbing) {
        CGDisplayHideCursor(kCGDirectMainDisplay);
        CGAssociateMouseAndMouseCursorPosition(false);
    }
    else {
        CGDisplayShowCursor(kCGDirectMainDisplay);
        CGAssociateMouseAndMouseCursorPosition(true);
    }
    return true;

    #elif defined(__linux__) || defined(__FreeBSD__)
        GdkWindow *window = gtk_widget_get_window(window::getHandle());
        GdkDisplay *display = gdk_window_get_display(window);
        GdkSeat *seat = gdk_display_get_default_seat(display);

        if(grabbing) {
        return gdk_seat_grab(
            seat,
            window,
            GDK_SEAT_CAPABILITY_POINTER,
            FALSE,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        ) == GDK_GRAB_SUCCESS;
    }
    else {
        gdk_seat_ungrab(seat);
        return true;
    }
    #else
    return false;
    #endif
}

bool sendKey(unsigned int keyCode, bool up = true) {
    #if defined(_WIN32)
    SHORT vk = VkKeyScanA(keyCode);

    INPUT in {};
    in.type = INPUT_KEYBOARD;
    in.ki.wVk = vk;
    in.ki.dwFlags = 0;

    SendInput(1, &in, sizeof(INPUT));

    if(up) {
        in.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &in, sizeof(INPUT));
    }
    return true;

    #elif defined(__APPLE__)
    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    CGEventRef ev = CGEventCreateKeyboardEvent(source, keyCode, true);
    CGEventPost(kCGHIDEventTap, ev);
    CFRelease(ev);

    if(up) {
        ev = CGEventCreateKeyboardEvent(source, keyCode, false);
        CGEventPost(kCGHIDEventTap, ev);
        CFRelease(ev);
    }
    return true;

    #elif defined(__linux__) || defined(__FreeBSD__)
    Display *d = XOpenDisplay(nullptr);
    if(!d) return false;

    XTestFakeKeyEvent(d, keyCode, true, CurrentTime);
    if(up) {
        XTestFakeKeyEvent(d, keyCode, false, CurrentTime);
    }
    XFlush(d);
    XCloseDisplay(d);
    return true;
    
    #else
    return false;
    #endif
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

json setMousePosition(const json &input) {
    json output;
    
    const auto missingRequiredField = helpers::missingRequiredField(input, {"x", "y"});
    if(missingRequiredField) {
        output["error"] = errors::makeMissingArgErrorPayload(missingRequiredField.value());
        return output;
    }

    int x = input["x"].get<int>();
    int y = input["y"].get<int>();

    if(!computer::setMousePosition(x, y)) {
        output["error"] = errors::makeErrorPayload(errors::NE_CO_UNLTOSC);
        return output;
    }

    output["success"] = true;
    return output;
}

json setMouseGrabbing(const json &input) {
    json output;

    bool grabbing = true;
    if(helpers::hasField(input, "grabbing"))
        grabbing = input["grabbing"].get<bool>();

    if(!computer::setMouseGrabbing(grabbing)) {
        output["error"] = errors::makeErrorPayload(errors::NE_CO_UNLTOMG);
        return output;
    }

    output["success"] = true;
    return output;
}

json sendKey(const json &input) {
    json output;
    
    const auto missingRequiredField = helpers::missingRequiredField(input, {"keyCode"});
    if(missingRequiredField) {
        output["error"] = errors::makeMissingArgErrorPayload(missingRequiredField.value());
        return output;
    }

    int keyCode = input["keyCode"].get<int>();
    bool up = true;
    
    if(helpers::hasField(input, "up"))
        up = input["up"].get<bool>();

    if(!computer::sendKey(keyCode, up)) {
        output["error"] = errors::makeErrorPayload(errors::NE_RT_NATRTER);
        return output;
    }

    output["success"] = true;
    return output;
}


} // namespace controllers
} // namespace computer
