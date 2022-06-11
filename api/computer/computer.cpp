#include <stdint.h>
#include <string>

#if defined(__linux__)
#include <sys/sysinfo.h>

#elif defined(__APPLE__) || defined(__FreeBSD__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#elif defined(_WIN32)
#define _WINSOCKAPI_
#include <windows.h>
#endif

#include <infoware/system.hpp>
#include <infoware/cpu.hpp>
#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace computer {

namespace controllers {

string __getArch(const iware::cpu::architecture_t &architecture) {
    switch(architecture) {
        case iware::cpu::architecture_t::x64:
            return "x64";
        case iware::cpu::architecture_t::arm:
            return "arm";
        case iware::cpu::architecture_t::itanium:
            return "itanium";
        case iware::cpu::architecture_t::x86:
            return "x86";
        default:
            return "unknown";
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
    const auto memory = iware::system::memory();

    output["returnValue"] = __getArch(iware::cpu::architecture());
    output["success"] = true;
    return output;
}

} // namespace controllers
} // namespace computer
