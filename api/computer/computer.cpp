#include <stdint.h>
#include <string>

#if defined(__linux__)
#include <sys/sysinfo.h>
#include <gdk/gdk.h>

#elif defined(__APPLE__) || defined(__FreeBSD__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#if defined(__APPLE__)
#include <CoreGraphics/CGEvent.h>
#endif

#elif defined(_WIN32)
#define _WINSOCKAPI_
#include <windows.h>
#endif

#include <infoware/system.hpp>
#include <infoware/cpu.hpp>
#include "api/computer/computer.h"
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

} // namespace controllers
} // namespace computer
