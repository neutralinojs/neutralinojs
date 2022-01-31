#include <string>

#include "server/neuserver.h"
#include "lib/json/json.hpp"
#include "helpers.h"
#include "extensions_loader.h"
#include "api/events/events.h"

using namespace std;
using json = nlohmann::json;

namespace extensions {

namespace controllers {

json dispatch(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"extensionId", "event"})) {
        output["error"] = helpers::makeMissingArgErrorPayload();
        return output;
    }
    string extensionId = input["extensionId"].get<string>();
    string event = input["event"].get<string>();
    json data = nullptr;

    if(helpers::hasField(input, "data")) {
        data = input["data"];
    }

    if(events::dispatchToExtension(extensionId, event, data)) {
        output["success"] = true;
    }
    else {
        output["error"] = helpers::makeErrorPayload("NE_EX_EXTNOTC",
                                    extensionId + " is not connected yet");
    }
    return output;
}

json broadcast(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"event"})) {
        output["error"] = helpers::makeMissingArgErrorPayload();
        return output;
    }
    string event = input["event"].get<string>();
    json data = nullptr;

    if(helpers::hasField(input, "data")) {
        data = input["data"];
    }

    events::dispatchToAllExtensions(event, data);
    output["success"] = true;

    return output;
}

json getStats(const json &input) {
    json output;
    json stats;
    stats["loaded"] = extensions::getLoaded();
    stats["connected"] = neuserver::getConnectedExtensions();

    output["returnValue"] = stats;
    output["success"] = true;
    return output;
}

} // namespace controllers
} // namespace extensions
