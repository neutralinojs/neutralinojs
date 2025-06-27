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
    id sharedPrintInfo = ((id(*)(id, SEL))objc_msgSend)("NSPrintInfo"_cls,
                                            "sharedPrintInfo"_sel);
    id printInfo = ((id(*)(id, SEL, id))objc_msgSend)((id)window::getWebviewHandle(),
                                            "printOperationWithPrintInfo:"_sel, sharedPrintInfo);
    ((void(*)(id, SEL, id, id, SEL, void(*)))objc_msgSend)(printInfo,
        "runOperationModalForWindow:delegate:didRunSelector:contextInfo:"_sel, (id)window::getWindowHandle(), nullptr, nullptr, nullptr);  
    #elif defined(_WIN32)
    
    #endif
    output["success"] = true;
    return output;
}

} // namespace controllers

} // namespace webview
