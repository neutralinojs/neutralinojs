#ifndef NEU_EVENT_H
#define NEU_EVENT_H

#include <string>

#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace events {
    void dispatch(const string &event, const json &data);
} // namespace events
#endif // #define NEU_EVENT_H
