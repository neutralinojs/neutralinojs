#ifndef NEU_DEBUG_H
#define NEU_DEBUG_H

#include <string>

#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace debug {
    void log(const string &type, const string &message);

namespace controllers {
    json log(const json &input);
} // namespace controllers
} // namespace debug

#endif // #define NEU_DEBUG_H
