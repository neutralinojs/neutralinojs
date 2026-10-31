#ifndef NEU_SINGLE_INSTANCE_H
#define NEU_SINGLE_INSTANCE_H

#include <string>

#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace single_instance {

enum class StartResult {
    Disabled,
    Primary,
    Forwarded,
    Error
};

StartResult start(const json &args, string &error);
void shutdown();
void onAppClientConnect();
void onAppClientDisconnect();
void onWindowReady();
void flushPendingEvents();

} // namespace single_instance

#endif // NEU_SINGLE_INSTANCE_H
