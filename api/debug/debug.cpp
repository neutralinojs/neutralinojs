#include <iostream>
#include <fstream>
#include "lib/json.hpp"
#include "lib/easylogging/easylogging++.h"

using namespace std;
using json = nlohmann::json;

namespace debug {
    void log(string type, string message) {
        if(type == "INFO")
            LOG(INFO) << message;
        else if(type == "ERROR")
            LOG(ERROR)  << message;
        else if(type == "WARNING")
            LOG(WARNING)  << message;
        else
            LOG(DEBUG) << message;
    }
    
namespace controllers {
    json log(json input) {
        json output;
        string type = input["type"];
        string message = input["message"];

        debug::log(type, message);
        output["message"] = "Wrote to the log file: neutralino.log";
        output["success"] = true;
        return output;
    }

} // namespace controllers
} // namespace debug
