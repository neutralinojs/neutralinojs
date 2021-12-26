#ifndef NEU_CHROME_H
#define NEU_CHROME_H

#include <string>
#include <vector>

#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace chrome {
    void init(const json &input);
} // namesapce chrome

#endif // #define NEU_CHROME_H

