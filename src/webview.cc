#define WEBVIEW_IMPLEMENTATION
#include "webview.h"

using namespace neut;

WebView::WebView(const string url, const string title, int width, int height,
                 int resizable) {
  _webview.url = url.c_str();
  _webview.title = title.c_str();
  _webview.width = width;
  _webview.height = height;
  _webview.resizable = resizable;

  webview_init(&_webview);
}

WebView::~WebView() { webview_exit(&_webview); }

void WebView::Run() {
  if (!running) {
    running = 1;
    while (webview_loop(&_webview, blocking) == 0) {
    }
  }
}

void WebView::showDialog(DialogType type, const string title, const string arg,
                         int flags, char *result, size_t resultsz) {
  webview_dialog_type t;
  switch (type) {
  case DialogType::AlertDialog:
    t = webview_dialog_type::WEBVIEW_DIALOG_TYPE_ALERT;
    break;
  case DialogType::OpenDialog:
    t = webview_dialog_type::WEBVIEW_DIALOG_TYPE_OPEN;
    break;
  case DialogType::SaveDialog:
    t = webview_dialog_type::WEBVIEW_DIALOG_TYPE_SAVE;
    break;
  }

  webview_dialog(&_webview, t, flags, title.c_str(), arg.c_str(), result,
                 resultsz);
}
