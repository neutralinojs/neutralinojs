#ifndef NEU_DEBUG_H
#define NEU_DEBUG_H

#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace debug {
    void log(string type, string message);

namespace controllers {
    json log(json input);
} // namespace controllers
} // namespace debug

#endif // #define NEU_DEBUG_H
