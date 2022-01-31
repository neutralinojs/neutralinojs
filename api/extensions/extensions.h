#ifndef NEU_EXT_H
#define NEU_EXT_H

#include <string>

#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace extensions {

namespace controllers {

json dispatch(const json &input);
json broadcast(const json &input);
json getStats(const json &input);

} // namespace controllers

} // namespace extensions

#endif // define NEU_EXT_H
