#ifndef NEU_NET_H
#define NEU_NET_H

#include <string>

#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace net {

namespace controllers {

json fetch(const json &input);

} // namespace controllers

} // namespace NET

#endif // #define NEU_NET_H
