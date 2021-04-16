#include "lib/json.hpp"
#include <windows.h>

using namespace std;
using json = nlohmann::json;

#define DIV 1024

namespace computer {
    string getRamUsage(json input) {
        json output;
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof (statex);

        GlobalMemoryStatusEx (&statex);
        output["ram"] = {
            {"total", statex.ullTotalPhys / DIV },
            {"available", statex.ullAvailPhys / DIV },
        };
        output["success"] = true;
        return output.dump();
    }
}
