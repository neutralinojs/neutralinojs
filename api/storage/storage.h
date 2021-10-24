#ifndef NEU_STORAGE_H
#define NEU_STORAGE_H

#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace storage {
namespace controllers {
    json setData(const json &input);
    json getData(const json &input);
} // namespace controllers
} // namespace storage

#endif // #define NEU_STORAGE_H
