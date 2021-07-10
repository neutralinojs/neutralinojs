#ifndef WI_H
#define WI_H
#include "lib/json.hpp"

#if defined(__APPLE__)
#include <objc/objc-runtime.h>

// Helpers to avoid too much typing with the Objective c runtime.
inline id operator"" _cls(const char *s, std::size_t) { return (id)objc_getClass(s); }
inline SEL operator"" _sel(const char *s, std::size_t) { return sel_registerName(s); }
inline id operator"" _str(const char *s, std::size_t) {
  return ((id(*)(id, SEL, const char *))objc_msgSend)(
      "NSString"_cls, "stringWithUTF8String:"_sel, s);
}
#endif

using json = nlohmann::json;
using namespace std;

namespace window {
    void executeJavaScript(string js);
    bool isMaximized();
    void maximize();
    void minimize();

namespace controllers {
    json show(json input);
    json setTitle(json input);
    json maximize(json input); 
    json unmaximize(json input);
    json isMaximized(json input);
    json minimize(json input);
        
} // namespace controllers
} // namespace window

#endif
