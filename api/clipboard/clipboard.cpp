#include <string>

#include "lib/json/json.hpp"
#include "lib/clip/clip.h"
#include "lib/base64/base64.hpp"
#include "helpers.h"
#include "errors.h"
#include "api/clipboard/clipboard.h"

using namespace std;
using json = nlohmann::json;

namespace clipboard {
namespace controllers {

json getFormat(const json &input) {
    json output;
    string format = "unknown";
    if(clip::has(clip::text_format())) {
        format = "text";
    }
    else if(clip::has(clip::image_format())) {
        format = "image";
    }
    else if(clip::has(clip::html_format())) {
        format = "html";
    }
    output["returnValue"] = format;
    output["success"] = true;
    return output;
}

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

json readImage(const json &input) {
    json output;
    json imgObj = nullptr;

    if(clip::has(clip::image_format())) {
        clip::image image;
        clip::image_spec spec;
        string clipData = "";

        clip::get_image(image);
        clip::get_image_spec(spec);
        size_t size = spec.required_data_size();
        clipData.resize(size);
        clipData.assign(image.data(), image.data() + size);

        imgObj = {
            { "width", spec.width },
            { "height", spec.height },
            { "bpp", spec.bits_per_pixel },
            { "bpr", spec.bytes_per_row },
            { "redMask", spec.red_mask },
            { "greenMask", spec.green_mask },
            { "blueMask", spec.blue_mask },
            { "redShift", spec.red_shift },
            { "greenShift", spec.green_shift },
            { "blueShift", spec.blue_shift },
            { "alphaMask", spec.alpha_mask },
            { "alphaShift", spec.alpha_shift },
            { "data", base64::to_base64(clipData) }
        };
    }

    output["returnValue"] = imgObj;
    output["success"] = true;
    return output;
}

json writeImage(const json &input) {
    json output;
    const auto missingRequiredField = helpers::missingRequiredField(input,
        { "width", "height", "bpp", "bpr", "redMask", "greenMask", "blueMask",
        "redShift", "greenShift", "blueShift", "data", "alphaShift", "alphaMask"});
    if (missingRequiredField) {
        output["error"] = errors::makeMissingArgErrorPayload(missingRequiredField.value());
        return output;
    }
    clip::image_spec spec;
    spec.width = input["width"].get<unsigned long>();
    spec.height = input["height"].get<unsigned long>();
    spec.bits_per_pixel = input["bpp"].get<unsigned long>();
    spec.bytes_per_row = input["bpr"].get<unsigned long>();
    spec.red_mask = input["redMask"].get<unsigned long>();
    spec.green_mask = input["greenMask"].get<unsigned long>();
    spec.blue_mask = input["blueMask"].get<unsigned long>();
    spec.red_shift = input["redShift"].get<unsigned long>();
    spec.green_shift = input["greenShift"].get<unsigned long>();
    spec.blue_shift = input["blueShift"].get<unsigned long>();
    spec.alpha_mask = input["alphaMask"].get<unsigned long>();
    spec.alpha_shift = input["alphaShift"].get<unsigned long>();

    string clipData = base64::from_base64(input["data"].get<string>());
    clip::image image(clipData.data(), spec);
    clip::set_image(image);

    output["message"] = "Image wrote to the system clipboard";
    output["success"] = true;
    return output;
}

json writeText(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"data"})) {
        output["error"] = errors::makeMissingArgErrorPayload("data");
        return output;
    }
    string data = input["data"].get<string>();
    clip::set_text(data);

    output["success"] = true;
    return output;
}

json readHTML(const json &input) {
    json output;
    string clipHTML = "";
    if(clip::has(clip::html_format())) {
        clip::get_html(clipHTML);
    }
    output["returnValue"] = clipHTML;
    output["success"] = true;
    return output;
}

json writeHTML(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"data"})) {
        output["error"] = errors::makeMissingArgErrorPayload("data");
        return output;
    }
    string data = input["data"].get<string>();
    clip::set_html(data);

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

} // namespace controllers
} // namespace clipboard
