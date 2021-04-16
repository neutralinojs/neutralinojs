#include <stdint.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include "lib/json.hpp"

using namespace std;
using json = nlohmann::json;

#define DIV 1024

namespace computer {
    string getRamUsage(json input) {
        json output;
        struct sysinfo sys_info;
        sysinfo(&sys_info);
        output["ram"] = {
            { "total", (sys_info.totalram * sys_info.mem_unit) / DIV },
            { "available", (sys_info.freeram * sys_info.mem_unit) / DIV }
        };
        output["success"] = true;
        return output.dump();
    }
}
