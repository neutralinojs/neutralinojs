#include <string>

#include "lib/json/json.hpp"
#include "helpers.h"
#include "server/neuserver.h"
#include "api/events/events.h"

using namespace std;
using json = nlohmann::json;

namespace extensions {

namespace controllers {
    json dispatch(const json &input) {
        json output;
        if(!input.contains("extensionId") || !input.contains("event")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        string extensionId = input["extensionId"].get<string>();
        string event = input["event"].get<string>();
        json data = nullptr;

        if(input.contains("data")) {
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
        if(!input.contains("event")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        string event = input["event"].get<string>();
        json data = nullptr;

        if(input.contains("data")) {
            data = input["data"];
        }

        events::dispatchToAllExtensions(event, data);
        output["success"] = true;

        return output;
    }

} // namespace controllers
} // namespace extensions
