#include <string>

#include "lib/json/json.hpp"
#include "lib/clip/clip.h"
#include "helpers.h"
#include "api/clipboard/clipboard.h"

using namespace std;
using json = nlohmann::json;

namespace clipboard {
namespace controllers {

    json readText(const json &input) {
        json output;
        string clipText = "";
        if(clip::has(clip::text_format())) {
            clip::get_text(clipText);
        }
        output["returnValue"] = clipText;
        output["success"] = true;
        return output;
    }

    json writeText(const json &input) {
        json output;
        if(!helpers::hasRequiredFields(input, {"data"})) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        string data = input["data"].get<string>();
        clip::set_text(data);

        output["success"] = true;
        return output;
    }

} // namespace controllers
} // namespace clipboard
