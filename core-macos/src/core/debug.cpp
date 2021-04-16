#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"
#include "../settings.h"
#include "log.h"

using namespace std;
using json = nlohmann::json;

namespace debug {
    string log(string jso) {
        json input;
        json output;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }
        string type = input["type"].get<std::string>();
        string message = input["message"].get<std::string>();

        if(type == "INFO")
            INFO() << message;
        else if(type == "ERROR")
            ERROR() << message;
        else if(type == "WARN")
            WARN() << message;
        else
            DEBUG() << message;

        output["message"] = "Wrote to log file neutralino.log";
        return output.dump();
    }

}
