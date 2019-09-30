#ifndef _WEBVIEW_H_
#define _WEBVIEW_H_

#include <cstdio>
#include <iostream>
#include <serge/webview.h>
#include <string>

using namespace std;

namespace neut {

typedef struct webview *wv;
typedef webview_dispatch_fn DispatcherFn;
typedef webview_external_invoke_cb_t CallbackFn;

enum class DialogType {
  AlertDialog,
  OpenDialog,
  SaveDialog,
};

class WebView {
private:
  struct webview _webview = {};
  int blocking = 0;
  int fullscreen = 0;
  int running = 0;

public:
  WebView(const string url, const string title, int width, int height,
          int resizable);
  ~WebView();
  void Run();
  void setBlocking(int blocking);
  int evalJS(const string js);
  int injectCSS(const string css);
  void setTitle(string title);
  void setFullscreen(int fullscreen);
  void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
  void showDialog(DialogType type, const string title, const string arg,
                  int flags, char *result, size_t resultsz);
  void dispatch(DispatcherFn fn, void *args);
  void setCallBack(CallbackFn fn);
  void terminate();
  void exit();
};

inline void WebView::setBlocking(int blocking) { blocking = blocking; }

inline void WebView::terminate() {
  running = 0;
  webview_terminate(&_webview);
}

inline void WebView::exit() {
  running = 0;
  webview_exit(&_webview);
}

inline int WebView::evalJS(const string js) {
  return webview_eval(&_webview, js.c_str());
}

inline int WebView::injectCSS(const string css) {
  return webview_inject_css(&_webview, css.c_str());
}

inline void WebView::setTitle(const string title) {
  webview_set_title(&_webview, title.c_str());
}

inline void WebView::setFullscreen(int fullscreen) {
  fullscreen = fullscreen;
  webview_set_fullscreen(&_webview, fullscreen);
}

inline void WebView::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  webview_set_color(&_webview, r, g, b, a);
}

inline void WebView::dispatch(DispatcherFn fn, void *args) {
  webview_dispatch(&_webview, fn, args);
}

inline void WebView::setCallBack(CallbackFn fn){
  _webview.external_invoke_cb = fn;
}

} // namespace neut

#endif
