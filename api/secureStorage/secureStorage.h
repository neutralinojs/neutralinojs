#ifndef NEU_SECURE_STORAGE_H
#define NEU_SECURE_STORAGE_H

#include "lib/json/json.hpp"

using json = nlohmann::json;

namespace secureStorage {

void init();

namespace controllers {

json setData(const json &input);
json getData(const json &input);

} // namespace controllers

} // namespace secureStorage

#endif // NEU_SECURE_STORAGE_H
