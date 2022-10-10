#include <string>
#include <vector>
#include <regex>

#include "helpers.h"
#include "errors.h"
#include "server/router.h"
#include "api/custom/custom.h"

#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace custom {
vector<string> getMethods() {
    auto methodMap = router::getMethodMap();
    vector<string> customMethods = {};
    for(const auto &[methodName, _]: methodMap) {
        if(methodName == "custom.getMethods") {
            continue;
        }

        if(regex_match(methodName, regex("^custom\\..*"))) {
            string cMethodName = regex_replace(methodName, regex("^custom\\."), "");
            customMethods.push_back(cMethodName);
        }
    }
    return customMethods;
}

namespace controllers {

json getMethods(const json &input) {
    json output;
    output["returnValue"] = custom::getMethods();
    output["success"] = true;
    return output;
}


/*

  Sample custom method.
  The client library will automatically add this method to the Neutralino global object.

  Usage examples:

  let sum;
  sum = await Neutralino.custom.add(10, 10); // 20
  sum = await Neutralino.custom.add(1, 1, { addExtraFive: true, addExtraTen: true }); // 17

*/

// json add(const json &input) {
//     json output;

//     // Validate
//     if(!helpers::hasRequiredFields(input, {"arg0", "arg1"})) {
//         output["error"] = errors::makeMissingArgErrorPayload();
//         return output;
//     }

//     // Extract input parameters
//     int a, b, sum = 0;
//     a = input["arg0"].get<int>();
//     b = input["arg1"].get<int>();

//     // Process
//     sum = a + b;

//     // Handle options
//     if(helpers::hasField(input, "addExtraFive")) {
//         if(input["addExtraFive"].get<bool>()) {
//             sum += 5;
//         }
//     }
//     if(helpers::hasField(input, "addExtraTen")) {
//         if(input["addExtraTen"].get<bool>()) {
//             sum += 10;
//         }
//     }

//     // Return the result
//     output["returnValue"] = sum;

//     // Mark the method call as a successful one
//     output["success"] = true;

//     return output;
// }

} // namespace controllers
} // namespace custom
