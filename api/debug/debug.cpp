#include <iostream>
#include <fstream>

#include "lib/json/json.hpp"
#include "lib/easylogging/easylogging++.h"
#include "helpers.h"

using namespace std;
using json = nlohmann::json;

namespace debug {
    void log(const string &type, const string &message) {
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
    json log(const json &input) {
        json output;
        string type = "INFO";
        if(!input.contains("message")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        if(input.contains("type")) {
            type = input["type"].get<string>();
        }

        string message = input["message"].get<string>();

        debug::log(type, message);
        output["message"] = "Wrote to the log file: neutralino.log";
        output["success"] = true;
        return output;
    }

} // namespace controllers
} // namespace debug
