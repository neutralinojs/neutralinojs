#include <string>

#include "lib/json/json.hpp"
#include "lib/clip/clip.h"
#include "helpers.h"
#include "errors.h"
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
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    string data = input["data"].get<string>();
    clip::set_text(data);

    output["success"] = true;
    return output;
}

json clear(const json &input) {
    json output;
    clip::clear();
    output["success"] = true;
    output["message"] = "Clipboard cleared";
    return output;
}

json writeImage(const json &input) {
    json output;
    std::vector<uint8_t> pixelData;
    const auto& dataArray = input["data"];
    if (dataArray.is_array()) {
        for (const auto &value : dataArray) {
            if (value.is_number_integer()) {
                pixelData.push_back(value.get<uint8_t>());
            }
        }
    } 
    clip::image_spec spec;
    spec.width = 3;
    spec.height = 2;
    clip::image img(pixelData.data(), spec);
    clip::set_image(img);
    output["success"] = true;
    return output;
}

} // namespace controllers
} // namespace clipboard
