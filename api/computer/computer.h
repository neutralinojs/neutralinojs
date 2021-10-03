#ifndef COMPUTER_H
#define COMPUTER_H

#include "lib/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace computer {
namespace controllers {
    json getMemoryInfo(json input);
} // namespace controllers
} // namespace computer

#endif
