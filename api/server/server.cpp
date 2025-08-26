#include <string>
#include <filesystem>

#include "lib/json/json.hpp"
#include "helpers.h"
#include "errors.h"
#include "server/router.h"
#include "api/server/server.h"

using namespace std;
using json = nlohmann::json;

namespace server {
namespace controllers {

json mount(const json &input) {
    json output;
    const auto missingRequiredField = helpers::missingRequiredField(input, {"path", "target"});
    if(missingRequiredField) {
        output["error"] = errors::makeMissingArgErrorPayload(missingRequiredField.value());
        return output;
    }
    
    string path = input["path"].get<string>();
    string target = input["target"].get<string>();
    errors::StatusCode mountStatus = router::mountPath(path, target);
    
    if(mountStatus != errors::NE_ST_OK) {
        output["error"] = errors::makeErrorPayload(mountStatus, target);
    }
    else {
        output["message"] = path + " mounts to " + target;
        output["success"] = true;
    }
    return output;
}

json unmount(const json &input) {
    json output;
    string path = input["path"].get<string>();

    if(!router::unmountPath(path)) {
        output["error"] = errors::makeErrorPayload(errors::NE_SR_NOMTPTH, path);
    }
    else {
        output["message"] = path + " unmounted";
        output["success"] = true;
    }
    return output;
}

json getMounts(const json &input) {
    json output;
    output["returnValue"] = router::getMounts();
    output["success"] = true;
    return output;
}

} // namespace controllers
} // namespace server
