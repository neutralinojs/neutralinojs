#ifndef NEU_SERVER_H
#define NEU_SERVER_H

#include <string>

#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace server {
namespace controllers {

json mount(const json &input);
json unmount(const json &input);
json getMounts(const json &input);


} // namespace controllers

} // namespace server

#endif // define NEU_SERVER_H
