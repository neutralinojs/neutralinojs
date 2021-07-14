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
    bool isVisible();
    void show();
    void hide();
    void setFullScreen();
    void exitFullScreen();
    bool isFullScreen();
    void setIcon(string icon);
    void setAwaysOnTop();
    void setBorderless();

namespace controllers {
    json init(json input);
    json setTitle(json input);
    json maximize(json input); 
    json unmaximize(json input);
    json isMaximized(json input);
    json minimize(json input);
    json isVisible(json input);
    json show(json input);
    json hide(json input);
    json setFullScreen(json input);
    json exitFullScreen(json input);
    json isFullScreen(json input);
    json focus(json input);
    json setIcon(json input);
        
} // namespace controllers
} // namespace window

#endif
