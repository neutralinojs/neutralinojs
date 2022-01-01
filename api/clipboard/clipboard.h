#ifndef NEU_CLIPBOARD_H
#define NEU_CLIPBOARD_H

#include <string>

#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace clipboard {
namespace controllers {
    json readText(const json &input);
    json writeText(const json &input);

} // namespace controllers
} // namespace clipboard
#endif // define NEU_CLIPBOARD_H
