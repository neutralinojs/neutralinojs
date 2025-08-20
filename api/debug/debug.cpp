#include <iostream>
#include <fstream>

#include "lib/json/json.hpp"
#include "lib/easylogging/easylogging++.h"
#include "helpers.h"
#include "errors.h"
#include "api/debug/debug.h"

using namespace std;
using json = nlohmann::json;

namespace debug {
void log(const debug::LogType type, const string &message) {
    switch(type) {
        case debug::LogTypeInfo:
            LOG(INFO) << message;
            break;
        case debug::LogTypeError:
            LOG(ERROR) << message;
            break;
        case debug::LogTypeWarning:
            LOG(WARNING) << message;
            break;
        default:
            LOG(DEBUG) << message;
            break;
    }
}

namespace controllers {

json log(const json &input) {
    json output;
    string type = "INFO";
    debug::LogType typeEn = debug::LogTypeInfo;

    if(!helpers::hasRequiredFields(input, {"message"})) {
        output["error"] = errors::makeMissingArgErrorPayload("message");
        return output;
    }
    if(helpers::hasField(input, "type")) {
        type = input["type"].get<string>();
    }

    string message = input["message"].get<string>();

    if(type == "ERROR") typeEn = debug::LogTypeError;
    if(type == "WARNING") typeEn = debug::LogTypeWarning;
    if(type == "DEBUG") typeEn = debug::LogTypeDebug;

    debug::log(typeEn, message);
    output["message"] = "Wrote to the log file: neutralino.log";
    output["success"] = true;
    return output;
}

} // namespace controllers
} // namespace debug
