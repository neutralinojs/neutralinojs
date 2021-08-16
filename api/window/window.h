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

    struct SizeOptions {
        int width = -1;
        int height = -1;
        int minWidth = -1;
        int minHeight = -1;
        int maxWidth = -1;
        int maxHeight = -1;
        bool resizable = true;
    };
    
    struct WindowOptions {
        SizeOptions sizeOptions;
        bool fullScreen = false;
        bool alwaysOnTop = false;
        bool enableInspector = false;
        bool borderless= false;
        bool maximize = false;
        bool hidden = false;
        bool maximizable = true;
        string title = "Neutralinojs window";
        string url = "https://neutralino.js.org";
        string icon = "";
    };
    
namespace handlers {
    void onClose();
} // namespace handlers

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
    
    void _close(int exitCode);

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
    json move(json input);
    json setSize(json input);
        
} // namespace controllers
} // namespace window

#endif
