#ifndef NEU_NET_H
#define NEU_NET_H

#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace net {

namespace controllers {

json request(const json &input);

} // namespace controllers

} // namespace net

#endif // #define NEU_NET_H
