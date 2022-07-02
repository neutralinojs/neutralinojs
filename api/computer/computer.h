#ifndef NEU_COMPUTER_H
#define NEU_COMPUTER_H

#include <string>

#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace computer {

namespace controllers {

json getMemoryInfo(const json &input);
json getArch(const json &input);
json getKernelInfo(const json &input);
json getOSInfo(const json &input);
json getDisplays(const json &input);

} // namespace controllers

} // namespace computer

#endif // #define NEU_COMPUTER_H
