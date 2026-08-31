#ifndef NEU_NET_H
#define NEU_NET_H

#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace net {

namespace controllers {

json request(const json &input);
json get(const json &input);
json post(const json &input);
json put(const json &input);
json del(const json &input);
json patch(const json &input);
json head(const json &input);
json options(const json &input);

} // namespace controllers

} // namespace net

#endif // #define NEU_NET_H
