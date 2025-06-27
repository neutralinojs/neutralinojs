#ifndef NEU_WEBVIEW_H
#define NEU_WEBVIEW_H

#include <string>

#include "errors.h"
#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace webview {
namespace controllers {

json print(const json &input);

} // namespace controllers

} // namespace webview

#endif // #define NEU_WEBVIEW_H
