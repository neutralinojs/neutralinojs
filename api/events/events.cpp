#include <string>

#include "server/neuserver.h"
#include "lib/json/json.hpp"
#include "helpers.h"
#include "errors.h"

using namespace std;
using json = nlohmann::json;

namespace events {

json __makeEventPayload(const string &event, const json &data) {
    json message;
    message["event"] = event;
    message["data"] = data;
    return message;
}

void dispatch(const string &event, const json &data) {
    neuserver::broadcast(__makeEventPayload(event, data));
}

void dispatchToAllExtensions(const string &event, const json &data) {
    neuserver::broadcastToAllExtensions(__makeEventPayload(event, data));
}

void dispatchToAllApps(const string &event, const json &data) {
    neuserver::broadcastToAllApps(__makeEventPayload(event, data));
}

bool dispatchToExtension(const string &extensionId, const string &event, const json &data) {
    return neuserver::sendToExtension(extensionId, __makeEventPayload(event, data));
}

namespace controllers {

json broadcast(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"event"})) {
        output["error"] = errors::makeMissingArgErrorPayload("event");
        return output;
    }
    string event = input["event"].get<string>();
    json data = nullptr;

    if(helpers::hasField(input, "data")) {
        data = input["data"];
    }

    events::dispatch(event, data);
    output["success"] = true;
    return output;
}
} // namespace controllers
} // namespace events
