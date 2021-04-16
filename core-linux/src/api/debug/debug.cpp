#include <iostream>
#include <fstream>
#include "lib/json.hpp"
#include "settings.h"
#include "log.h"

using namespace std;
using json = nlohmann::json;

namespace debug {
    string log(json input) {
        json output;
        string type = input["type"];
        string message = input["message"];

        if(type == "INFO")
            INFO() << message;
        else if(type == "ERROR")
            ERROR() << message;
        else if(type == "WARN")
            WARN() << message;
        else
            DEBUG() << message;

        output["message"] = "Wrote to the log file: neutralino.log";
        output["success"] = true;
        return output.dump();
    }

}
