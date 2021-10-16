#ifndef EVT_H
#define EVT_H

#include <string>

#include "lib/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace events {
    void dispatch(string event, json data);
} // namespace events
#endif
