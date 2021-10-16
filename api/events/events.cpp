#include <string>

#include "server/neuserver.h"

#include "lib/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace events {
    void dispatch(string event, json data) {
        json message;
        message["event"] = event;
        message["data"] = data;
        neuserver::broadcast(message);
    }
} // namespace events
