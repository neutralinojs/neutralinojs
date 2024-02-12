#include <iostream>
#include <fstream>
#include <iostream>

#include "lib/json/json.hpp"
#include "helpers.h"
#include "errors.h"
#include "api/debug/debug.h"

using namespace std;
using json = nlohmann::json;

namespace debug {

void log(const debug::LogType type, const string &message) {
    
    cout << type << message << endl;
    cout.flush();
}

namespace controllers {

json log(const json &input) {
    json output;
    string type = "";

    if(!helpers::hasRequiredFields(input, {"message"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    
    if(helpers::hasField(input, "type")) {
        type = input["type"].get<string>();
    }

    string message = input["message"].get<string>();

    if (type == "ERROR")
    {
        cerr << type << message << endl;
        cerr.flush();
    }
    else
    {
        cout << type << message << endl;
        cout.flush();
    }
    
    output["message"] = "Wrote to the log file: neutralino.log";
    output["success"] = true;
    return output;
}

} // namespace controllers
} // namespace debug
