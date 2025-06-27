#include <string>

#include "lib/json/json.hpp"
#include "helpers.h"
#include "errors.h"
#include "api/window/window.h"
using namespace std;
using json = nlohmann::json;

namespace webview {
namespace controllers {

json print(const json &input) {
    json output;
    #if defined(__linux__) || defined(__FreeBSD__)
    
    #elif defined(__APPLE__)
    
    #elif defined(_WIN32)
    
    #endif
    output["success"] = true;
    return output;
}

} // namespace controllers

} // namespace webview
