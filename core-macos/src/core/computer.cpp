#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include "nlohmann/json.hpp"
#include "include/computer.h"
#include "sysstat.h"

using namespace std;
using namespace SystemStat;
using json = nlohmann::json;

#define DIV 1024

namespace computer {
    string getRamUsage(string jso) {
        json output;
        MemoryStat memstat;
        getMemoryStat(&memstat);
        output["ram"] = {
            { "total", (memstat.phys.total) / DIV / DIV },
            { "available", (memstat.phys.total - memstat.phys.avail) / DIV }
        };

        return output.dump();
    }
}
