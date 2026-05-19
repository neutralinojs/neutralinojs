#include <stdint.h>
#include <string>
#include <algorithm>
#include "helpers.h"
#include "errors.h"

#if defined(__linux__)
#include <sys/sysinfo.h>
#include <unistd.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <cstdlib>
#include <fstream>

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
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <IOKit/IOKitLib.h>

#elif defined(_WIN32)
#define _WINSOCKAPI_
#include <windows.h>
#include <dxgi.h>
#pragma comment(lib, "dxgi.lib")
#endif


#include <infoware/system.hpp>
#include <infoware/cpu.hpp>
#include "api/computer/computer.h"
#include "helpers.h"
#include "api/window/window.h"
#include "api/os/os.h"
#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace computer {

#if defined(__linux__) || defined(__FreeBSD__)
bool __isWayland() {
    return os::getEnv("XDG_SESSION_TYPE") == "wayland";
}
#endif

#if defined(__APPLE__)
CFMachPortRef mouseTap = nullptr;
CFRunLoopSourceRef runLoopSource = nullptr;

CGEventRef __mouseTapCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
    long winId = ((long(*)(id, SEL))objc_msgSend)(window::getHandle(), "windowNumber"_sel);
    auto winInfoArray = CGWindowListCopyWindowInfo(kCGWindowListOptionIncludingWindow, winId);
    auto winInfo = CFArrayGetValueAtIndex(winInfoArray, 0);
    auto winBounds = (CFDictionaryRef)CFDictionaryGetValue((CFDictionaryRef) winInfo, kCGWindowBounds);

    CGRect winRect = CGRectZero;
    CGRectMakeWithDictionaryRepresentation(winBounds, &winRect);

    if(CGRectIsEmpty(winRect)) return event;

    if(type == kCGEventMouseMoved || type == kCGEventLeftMouseDragged || type == kCGEventRightMouseDragged) {
        CGPoint location = CGEventGetLocation(event);
        CGFloat minX = winRect.origin.x;
        CGFloat maxX = winRect.origin.x + winRect.size.width;
        CGFloat minY = winRect.origin.y;
        CGFloat maxY = winRect.origin.y + winRect.size.height;

        CGFloat clampedX = max(minX, min(location.x, maxX));
        CGFloat clampedY = max(minY, min(location.y, maxY));

        if(location.x != clampedX || location.y != clampedY) {
            auto point = CGPointMake(clampedX, clampedY);
            CGEventSetLocation(event, point);
            CGWarpMouseCursorPosition(point);
        }
    }
    return event;
}
#endif

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

#if defined(_WIN32)

string __getGPUVendor(unsigned int vendorId) {
    switch(vendorId) {
        case 0x10DE:
            return "NVIDIA Corporation";

        case 0x1002:
        case 0x1022:
            return "Advanced Micro Devices, Inc.";

        case 0x8086:
            return "Intel Corporation";

        default:
            return "Unknown";
    }
}

#endif

#if defined(__linux__)

string __parseLinuxGPUVendor(const string &vendorId) {
    if(vendorId == "0x10de")
        return "NVIDIA Corporation";

    if(vendorId == "0x1002")
        return "Advanced Micro Devices, Inc.";

    if(vendorId == "0x8086")
        return "Intel Corporation";

    return "Unknown";
}

#endif


json getGPUInfo() {
    json output = {
        { "name", "" },
        { "vendor", "" },
        { "vramSize", 0 },
        { "driverVersion", "" }
    };

    #if defined(_WIN32)

    IDXGIFactory* factory = nullptr;

    if(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory) != S_OK)
        return output;

    IDXGIAdapter* adapter = nullptr;

    if(factory->EnumAdapters(0, &adapter) != S_OK) {
        factory->Release();
        return output;
    }

    DXGI_ADAPTER_DESC desc;
    adapter->GetDesc(&desc);

    string gpuName = helpers::wstr2str(desc.Description);

    output["name"] = gpuName;
    output["vendor"] = __getGPUVendor(desc.VendorId);

    output["vramSize"] =
        static_cast<uint64_t>(
            desc.DedicatedVideoMemory / (1024 * 1024)
        );

    LARGE_INTEGER driverVersion;
    if(adapter->CheckInterfaceSupport(
        __uuidof(IDXGIDevice),
        &driverVersion
    ) == S_OK) {

        output["driverVersion"] =
            to_string(HIWORD(driverVersion.HighPart)) + "." +
            to_string(LOWORD(driverVersion.HighPart));
    }

    adapter->Release();
    factory->Release();

    #elif defined(__linux__)

    string vendorId = "";

    ifstream vendorFile("/sys/class/drm/card0/device/vendor");

    if(vendorFile.is_open()) {
        getline(vendorFile, vendorId);
        vendorFile.close();
    }

    output["vendor"] = __parseLinuxGPUVendor(vendorId);

    ifstream nameFile("/sys/class/drm/card0/device/product_name");

    if(nameFile.is_open()) {
        string gpuName;
        getline(nameFile, gpuName);
        output["name"] = gpuName;
        nameFile.close();
    }

    if(output["name"] == "") {
        ifstream ueventFile("/sys/class/drm/card0/device/uevent");
        if(ueventFile.is_open()) {
            string line;
            while(getline(ueventFile, line)) {
                if(line.find("DRIVER=") == 0) {
                    output["name"] = line.substr(7);
                    break;
                }
            }
            ueventFile.close();
        }
    }


    
    ifstream vramFile("/sys/class/drm/card0/device/mem_info_vram_total");

    if(vramFile.is_open()) {

        uint64_t vramBytes = 0;

        vramFile >> vramBytes;

        output["vramSize"] =
            static_cast<uint64_t>(
                vramBytes / (1024 * 1024)
            );

        vramFile.close();
    }

    #elif defined(__APPLE__)

    io_iterator_t iterator;
    io_service_t service;

    CFMutableDictionaryRef matchDict =
        IOServiceMatching("IOPCIDevice");

    if(IOServiceGetMatchingServices(
        kIOMainPortDefault,
        matchDict,
        &iterator
    ) == KERN_SUCCESS) {

        while((service = IOIteratorNext(iterator))) {

            CFTypeRef model =
                IORegistryEntryCreateCFProperty(
                    service,
                    CFSTR("model"),
                    kCFAllocatorDefault,
                    0
                );

            if(model) {

                CFDataRef data = (CFDataRef)model;

                string gpuName(
                    (const char*)CFDataGetBytePtr(data),
                    CFDataGetLength(data)
                );

                gpuName.erase(
                    find(gpuName.begin(),gpuName.end(),'\0'),
                    gpuName.end()
                );

                output["name"] = gpuName;
                output["vendor"] = "Apple";

                CFRelease(model);

                IOObjectRelease(service);
                break;
            }

            IOObjectRelease(service);
        }

        IOObjectRelease(iterator);
    }

    #endif

    return output;
}

bool setMousePosition(int x, int y) {
    #if defined(_WIN32)
    POINT pos { x, y };
    return SetCursorPos(pos.x, pos.y);

    #elif defined(__APPLE__)
    CGPoint pos = CGPointMake(x, y);
    CGWarpMouseCursorPosition(pos);
    return true;

    #elif defined(__linux__) || defined(__FreeBSD__)
    if(__isWayland()) return false;
    
    Display *display = XOpenDisplay(nullptr);
    if(!display) return false;

    XWarpPointer(display, None, DefaultRootWindow(display), 0, 0, 0, 0, x, y);
    XFlush(display);
    XCloseDisplay(display);
    return true;
    #else
    return false;
    #endif
}

bool setMouseGrabbing(bool grabbing = true) {
    #if defined(_WIN32)
    HWND hwnd = window::getHandle();

    if(grabbing) {
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        POINT topLeft {clientRect.left, clientRect.top}, bottomRight {clientRect.right, clientRect.bottom};
        ClientToScreen(hwnd, &topLeft);
        ClientToScreen(hwnd, &bottomRight);
        RECT clip {topLeft.x, topLeft.y, bottomRight.x, bottomRight.y};
        return ClipCursor(&clip);
    }
    return ClipCursor(nullptr);

    #elif defined(__APPLE__)
    if(grabbing) {
        mouseTap = CGEventTapCreate(
            kCGSessionEventTap, 
            kCGHeadInsertEventTap, 
            kCGEventTapOptionDefault, 
            CGEventMaskBit(kCGEventMouseMoved) | CGEventMaskBit(kCGEventLeftMouseDragged) | CGEventMaskBit(kCGEventRightMouseDragged), 
            __mouseTapCallback, 
            nullptr
        );

        if(!mouseTap) return false;

        runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, mouseTap, 0);
        CFRunLoopAddSource(CFRunLoopGetMain(), runLoopSource, kCFRunLoopCommonModes);
        CGEventTapEnable(mouseTap, true);
    }
    else {
        if(!mouseTap) return false;

        CGEventTapEnable(mouseTap, false);
        if(runLoopSource) {
            CFRunLoopRemoveSource(CFRunLoopGetMain(), runLoopSource, kCFRunLoopCommonModes);
            CFRunLoopSourceInvalidate(runLoopSource);
            CFRelease(runLoopSource);
            runLoopSource = nullptr;
        }
        CFRelease(mouseTap);
        mouseTap = nullptr;
    }
    return true;

    #elif defined(__linux__) || defined(__FreeBSD__)
    if(__isWayland()) return false;
    
    GdkWindow *gdkWindow = gtk_widget_get_window(window::getHandle());
    Display *xDisplay = gdk_x11_display_get_xdisplay(gdk_window_get_display(gdkWindow));
    Window xWindow = gdk_x11_window_get_xid(gdkWindow);

    if(grabbing) {
        return XGrabPointer(
            xDisplay,
            xWindow,
            True,
            ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
            GrabModeAsync,
            GrabModeAsync,
            xWindow,
            None,
            CurrentTime
        ) == GrabSuccess;
    }
    else {
        XUngrabPointer(xDisplay, CurrentTime);
        return true;
    }
    #else
    return false;
    #endif
}

bool sendKey(unsigned int keyCode, computer::SendKeyState keyState = computer::SendKeyStatePress) {
    #if defined(_WIN32)
    INPUT in {};
    in.type = INPUT_KEYBOARD;
    in.ki.wVk = keyCode;

    if(keyState == computer::SendKeyStatePress || keyState == computer::SendKeyStateDown) {
        in.ki.dwFlags = 0;
        SendInput(1, &in, sizeof(INPUT));
    }

    if(keyState == computer::SendKeyStatePress || keyState == computer::SendKeyStateUp) {
        in.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &in, sizeof(INPUT));
    }
    return true;

    #elif defined(__APPLE__)
    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);

    if(keyState == computer::SendKeyStatePress || keyState == computer::SendKeyStateDown) {
        CGEventRef event = CGEventCreateKeyboardEvent(source, keyCode, true);
        CGEventPost(kCGHIDEventTap, event);
        CFRelease(event);
    }

    if(keyState == computer::SendKeyStatePress || keyState == computer::SendKeyStateUp) {
        CGEventRef event = CGEventCreateKeyboardEvent(source, keyCode, false);
        CGEventPost(kCGHIDEventTap, event);
        CFRelease(event);
    }
    return true;

    #elif defined(__linux__) || defined(__FreeBSD__)
    if(__isWayland()) return false;
    
    Display *display = XOpenDisplay(nullptr);
    if(!display) return false;

    if(keyState == computer::SendKeyStatePress || keyState == computer::SendKeyStateDown) {
        XTestFakeKeyEvent(display, keyCode, true, CurrentTime);
    }
    if(keyState == computer::SendKeyStatePress || keyState == computer::SendKeyStateUp) {
        XTestFakeKeyEvent(display, keyCode, false, CurrentTime);
    }
    XFlush(display);
    XCloseDisplay(display);
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

json getGPUInfo(const json &input) {
    json output;

    output["returnValue"] = computer::getGPUInfo();
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

json getHostname(const json &input) {
    json output;
    string hostname = "";

    #if defined(_WIN32)
    wstring hostnameW;
    hostnameW.resize(MAX_COMPUTERNAME_LENGTH + 1);
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    if(GetComputerName(hostnameW.data(), &size)) {
        hostnameW.resize(size);
        hostname = helpers::wstr2str(hostnameW);
    }
    #else
    char hostnameBuffer[256] = { 0 };
    if(gethostname(hostnameBuffer, sizeof(hostnameBuffer)) == 0) {
        hostname = string(hostnameBuffer);
    }
    #endif

    output["returnValue"] = hostname;
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
    
    const auto missingRequiredField = helpers::missingRequiredField(input, {"key"});
    if(missingRequiredField) {
        output["error"] = errors::makeMissingArgErrorPayload(missingRequiredField.value());
        return output;
    }

    int key = input["key"].get<int>();
    computer::SendKeyState state = computer::SendKeyStatePress;
    
    if(helpers::hasField(input, "state")) {
        string st = input["state"].get<string>();
        if(st == "press") state = computer::SendKeyStatePress;
        if(st == "down") state = computer::SendKeyStateDown;
        if(st == "up") state = computer::SendKeyStateUp;
    }

    if(!computer::sendKey(key, state)) {
        output["error"] = errors::makeErrorPayload(errors::NE_CO_UNLTOSK);
        return output;
    }

    output["success"] = true;
    return output;
}


} // namespace controllers
} // namespace computer
