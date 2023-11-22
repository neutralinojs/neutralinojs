#ifndef NEU_WINDOW_H
#define NEU_WINDOW_H

#include <string>

#include "errors.h"
#include "lib/json/json.hpp"

#if defined(__linux__) || defined(__FreeBSD__)
#include <gtk/gtk.h>
#define NEU_W_HANDLE GtkWidget*

#elif defined(__APPLE__)
#include <objc/objc-runtime.h>

// Helpers to avoid too much typing with the Objective C runtime.
inline id operator"" _cls(const char *s, size_t) { return (id)objc_getClass(s); }
inline SEL operator"" _sel(const char *s, size_t) { return sel_registerName(s); }
inline id operator"" _str(const char *s, size_t) {
  return ((id(*)(id, SEL, const char *))objc_msgSend)(
      "NSString"_cls, "stringWithUTF8String:"_sel, s);
}
#define NEU_W_HANDLE id

#elif defined(_WIN32)
#include <windows.h>
#define NEU_W_HANDLE HWND
#endif

#define NEU_WIN_CONFIG_FILE "/.tmp/window_state.config.json"

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
    bool center = false;
    bool maximizable = true;
    bool exitProcessOnClose = true;
    bool useSavedState = true;
    string title = "Neutralinojs window";
    string url = "https://neutralino.js.org";
    string icon = "";
    string extendUserAgentWith = "";
    int x = 0;
    int y = 0;
};

namespace handlers {

void onClose();

} // namespace handlers

NEU_W_HANDLE getWindowHandle();
bool isSavedStateLoaded();
void executeJavaScript(const string &js);
bool isMaximized();
void maximize();
void minimize();
bool isVisible();
bool isFakeHidden();
void undoFakeHidden();
void show();
void hide();
void setFullScreen();
void exitFullScreen();
bool isFullScreen();
void setIcon(const string &icon);
void move(int x, int y);
window::SizeOptions getSize();
pair<int, int> getPosition();
void center(bool useConfigSizes);
void setAlwaysOnTop(bool onTop);
void setBorderless();

void _close(int exitCode);

namespace controllers {

json init(const json &input);
json setTitle(const json &input);
json getTitle(const json &input);
json maximize(const json &input);
json unmaximize(const json &input);
json isMaximized(const json &input);
json minimize(const json &input);
json isVisible(const json &input);
json show(const json &input);
json hide(const json &input);
json setFullScreen(const json &input);
json exitFullScreen(const json &input);
json isFullScreen(const json &input);
json focus(const json &input);
json setIcon(const json &input);
json move(const json &input);
json center(const json &input);
json setSize(const json &input);
json getSize(const json &input);
json getPosition(const json &input);
json setAlwaysOnTop(const json &input);

} // namespace controllers

} // namespace window

#endif // #define NEU_WINDOW_H
