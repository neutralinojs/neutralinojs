#ifndef NEU_EVENT_H
#define NEU_EVENT_H

#include <string>

#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace events {
    void dispatch(const string &event, const json &data); // notifies all clients
    void dispatchToAllExtensions(const string &event, const json &data); // notifies all ext clients
    void dispatchToAllApps(const string &event, const json &data); // notifies all app clients
    bool dispatchToExtension(const string &extensionId, const string &event, const json &data);
    //notifies specific ext client

namespace controllers {
    json broadcast(const json &input);
} // namespace controllers
} // namespace events
#endif // #define NEU_EVENT_H
