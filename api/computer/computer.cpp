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

#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

#define DIV 1024

namespace computer {
namespace controllers {
    json getMemoryInfo(const json &input) {
        json output;
        #if defined(__linux__)
        struct sysinfo sys_info;
        sysinfo(&sys_info);
        output["returnValue"] = {
            { "total", (sys_info.totalram * sys_info.mem_unit) / DIV },
            { "available", (sys_info.freeram * sys_info.mem_unit) / DIV }
        };
        
        #elif defined(__APPLE__) || defined(__FreeBSD__)
        long pages = sysconf(_SC_PHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);

        int mib[2];
        int64_t physical_memory;
        mib[0] = CTL_HW;

	#if defined(__FreeBSD__)
	#define HW_MEMSIZE HW_PHYSMEM
	#endif
        mib[1] = HW_MEMSIZE;
        size_t length = sizeof(int64_t);
        sysctl(mib, 2, &physical_memory, &length, NULL, 0);
        output["returnValue"] = {
            { "total", (pages * page_size) / DIV },
            { "available", 0 / DIV } // TODO: implement
        };
        
        #elif defined(_WIN32)
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof (statex);

        GlobalMemoryStatusEx (&statex);
        output["returnValue"] = {
            {"total", statex.ullTotalPhys / DIV },
            {"available", statex.ullAvailPhys / DIV },
        };
        #endif
        output["success"] = true;
        return output;
    }
} // namespace controllers
} // namespace computer
