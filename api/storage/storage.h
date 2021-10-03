#ifndef STORAGE_H
#define STORAGE_H

#include "lib/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace storage {
namespace controllers {
    json setData(json input);
    json getData(json input);
} // namespace controllers
} // namespace storage

#endif
