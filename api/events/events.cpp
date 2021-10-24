#include <string>

#include "server/neuserver.h"
#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace events {
    void dispatch(const string &event, const json &data) {
        json message;
        message["event"] = event;
        message["data"] = data;
        neuserver::broadcast(message);
    }
} // namespace events
