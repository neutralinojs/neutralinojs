#ifndef NEU_RES_H
#define NEU_RES_H

#include <string>

#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace res {



namespace controllers {

json getFiles(const json &input);
json getStats(const json &input);
json extractFile(const json &input);
json extractDirectory(const json &input);
json readFile(const json &input);
json readBinaryFile(const json &input);

} // namespace controllers

} // namespace res

#endif // define NEU_RES_H
