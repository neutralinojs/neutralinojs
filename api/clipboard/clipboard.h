#ifndef NEU_CLIPBOARD_H
#define NEU_CLIPBOARD_H

#include <string>

#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace clipboard {

namespace controllers {

json getFormat(const json &input);
json readText(const json &input);
json readImage(const json &input);
json writeText(const json &input);
json writeImage(const json &input);
json writeHTML(const json &input);
json readHTML(const json &input);
json clear(const json &input);

} // namespace controllers

} // namespace clipboard

#endif // define NEU_CLIPBOARD_H
