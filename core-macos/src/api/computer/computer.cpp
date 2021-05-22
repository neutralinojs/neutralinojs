#include <stdint.h>
#include <stdio.h>
#include "lib/json.hpp"

using namespace std;
using json = nlohmann::json;

#define DIV 1024

namespace computer {
    string getRamUsage(json input) {
        json output;
        // TODO: implement on macos
        output["ram"] = {
            { "total", 0 / DIV },
            { "available", 0 / DIV }
        };
        output["success"] = true;
        return output.dump();
    }
}
