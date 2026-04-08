#include <stdint.h>
#include <string>
#include <map>

#if defined(__linux__)
#include <sys/sysinfo.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <cstdlib>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netpacket/packet.h>

#elif defined(__FreeBSD__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if_dl.h>

#elif defined(__APPLE__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if_dl.h>

#elif defined(_WIN32)
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _HAS_STD_BYTE 0
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#endif

#include "helpers.h"
#include "errors.h"


#include <infoware/system.hpp>
#include <infoware/cpu.hpp>
#include "api/computer/computer.h"
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

json getNetworkInterfaces() {
    json interfaces = json::array();
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        return interfaces;
    }
    std::map<std::string, json> ifaceMap;

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        
        std::string name = ifa->ifa_name;
        if (ifaceMap.find(name) == ifaceMap.end()) {
            ifaceMap[name] = {{"name", name}, {"ipv4", ""}, {"ipv6", ""}, {"mac", ""}};
        }

        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET) {
            char ip[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr, ip, INET_ADDRSTRLEN)) {
                ifaceMap[name]["ipv4"] = string(ip);
            }
        } else if (family == AF_INET6) {
            char ip[INET6_ADDRSTRLEN];
            if (inet_ntop(AF_INET6, &((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr, ip, INET6_ADDRSTRLEN)) {
                ifaceMap[name]["ipv6"] = string(ip);
            }
        }
#if defined(__linux__)
        else if (family == AF_PACKET) {
            struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;
            if (s->sll_halen == 6) {
                char mac[18];
                snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
                    s->sll_addr[0], s->sll_addr[1], s->sll_addr[2],
                    s->sll_addr[3], s->sll_addr[4], s->sll_addr[5]);
                ifaceMap[name]["mac"] = string(mac);
            }
        }
#elif defined(__APPLE__) || defined(__FreeBSD__)
        else if (family == AF_LINK) {
            struct sockaddr_dl *sdl = (struct sockaddr_dl *)ifa->ifa_addr;
            if (sdl->sdl_alen == 6) {
                unsigned char *mac_ptr = (unsigned char *)LLADDR(sdl);
                char mac[18];
                snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
                    mac_ptr[0], mac_ptr[1], mac_ptr[2],
                    mac_ptr[3], mac_ptr[4], mac_ptr[5]);
                ifaceMap[name]["mac"] = string(mac);
            }
        }
#endif
    }
    freeifaddrs(ifaddr);

    for (const auto& kv : ifaceMap) {
        interfaces.push_back(kv.second);
    }
#elif defined(_WIN32)
    ULONG outBufLen = 15000;
    PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);
    if (!pAddresses) return interfaces;

    DWORD dwRetVal = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, pAddresses, &outBufLen);
    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        free(pAddresses);
        pAddresses = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);
        if (!pAddresses) return interfaces;
        dwRetVal = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, pAddresses, &outBufLen);
    }

    if (dwRetVal == NO_ERROR) {
        PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
        while (pCurrAddresses) {
            json iface = {
                {"name", helpers::wcstr2str(pCurrAddresses->FriendlyName)},
                {"ipv4", ""},
                {"ipv6", ""},
                {"mac", ""}
            };

            PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddresses->FirstUnicastAddress;
            while (pUnicast != nullptr) {
                if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
                    char ip[INET_ADDRSTRLEN];
                    if (inet_ntop(AF_INET, &((struct sockaddr_in*)pUnicast->Address.lpSockaddr)->sin_addr, ip, INET_ADDRSTRLEN)) {
                        iface["ipv4"] = string(ip);
                    }
                } else if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6) {
                    char ip[INET6_ADDRSTRLEN];
                    if (inet_ntop(AF_INET6, &((struct sockaddr_in6*)pUnicast->Address.lpSockaddr)->sin6_addr, ip, INET6_ADDRSTRLEN)) {
                        iface["ipv6"] = string(ip);
                    }
                }
                pUnicast = pUnicast->Next;
            }

            if (pCurrAddresses->PhysicalAddressLength == 6) {
                char mac[18];
                snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
                    pCurrAddresses->PhysicalAddress[0], pCurrAddresses->PhysicalAddress[1],
                    pCurrAddresses->PhysicalAddress[2], pCurrAddresses->PhysicalAddress[3],
                    pCurrAddresses->PhysicalAddress[4], pCurrAddresses->PhysicalAddress[5]);
                iface["mac"] = string(mac);
            }

            interfaces.push_back(iface);
            pCurrAddresses = pCurrAddresses->Next;
        }
    }
    free(pAddresses);
#endif
    return interfaces;
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

json getNetworkInterfaces(const json &input) {
    json output;
    output["returnValue"] = computer::getNetworkInterfaces();
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
