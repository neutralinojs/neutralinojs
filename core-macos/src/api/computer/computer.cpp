#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include "lib/json.hpp"

using namespace std;
using json = nlohmann::json;

#define DIV 1024

namespace computer {
    string getRamUsage(json input) {
        json output;
        long pages = sysconf(_SC_PHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);

        int mib[2];
        int64_t physical_memory;
        mib[0] = CTL_HW;
        mib[1] = HW_MEMSIZE;
        size_t length = sizeof(int64_t);
        sysctl(mib, 2, &physical_memory, &length, NULL, 0);
        output["ram"] = {
            { "total", (pages * page_size) / DIV },
            { "available", 0 / DIV } // TODO: implement
        };
        output["success"] = true;
        return output.dump();
    }
}
