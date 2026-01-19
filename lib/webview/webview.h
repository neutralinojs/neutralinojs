/*
 * MIT License
 *
 * Copyright (c) 2017 Serge Zaitsev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef WEBVIEW_H
#define WEBVIEW_H

// Window size hints
#define WEBVIEW_HINT_NONE 0  // Width and height are default size
#define WEBVIEW_HINT_MIN 1   // Width and height are minimum bounds
#define WEBVIEW_HINT_MAX 2   // Width and height are maximum bounds
#define WEBVIEW_HINT_FIXED 3 // Window size can not be changed by a user
// Window events
#define WEBVIEW_WINDOW_CLOSE 0
#define WEBVIEW_WINDOW_FOCUS 1
#define WEBVIEW_WINDOW_BLUR 2
#define WEBVIEW_WINDOW_FULLSCREEN 3
#define WEBVIEW_WINDOW_UNFULLSCREEN 4
#define WEBVIEW_WINDOW_MINIMIZED 5
#define WEBVIEW_WINDOW_RESTORED 6
#define WEBVIEW_WINDOW_SHOW 7
#define WEBVIEW_WINDOW_HIDE 8
#define WEBVIEW_WINDOW_MAXIMIZE 9
#define WEBVIEW_WINDOW_UNDEFINED 100

#ifndef WEBVIEW_HEADER

#if !defined(WEBVIEW_GTK) && !defined(WEBVIEW_COCOA) && !defined(WEBVIEW_EDGE)
#if defined(__linux__)
#define WEBVIEW_GTK
#elif defined(__APPLE__)
#define WEBVIEW_COCOA
#elif defined(_WIN32)
#define WEBVIEW_EDGE
#else
#error "please, specify webview backend"
#endif
#endif

#include <atomic>
#include <functional>
#include <future>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <type_traits>
#include <cstring>

namespace webview {
using dispatch_fn_t = std::function<void()>;
using eventHandler_t = std::function<void(int)>;

static eventHandler_t windowStateChange;
static int processExitCode = 0;

struct WindowMenuItem {
  std::string id;
  std::string text;
  bool disabled = false;
  bool checked = false;
  std::string action = "menuCallback:";
  std::string shortcut;

  void (*cb)(struct WindowMenuItem *);
};

} // namespace webview

#if defined(WEBVIEW_GTK)
//
// ====================================================================
//
// This implementation uses webkit2gtk backend. It requires gtk+3.0 and
// webkit2gtk-4.0 or webkit2gtk-4.1 libraries. Proper compiler flags can be retrieved via:
//
//   pkg-config --cflags --libs gtk+-3.0
//
// ====================================================================
//
#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkscreen.h>
#include <cairo/cairo.h>
#include <dlfcn.h>


// webkit2gtk definitions
using WebKitWebView = struct _WebKitWebView;
using WebKitSettings = struct _WebKitSettings;
using WebKitWebInspector = struct _WebKitWebInspector;
using WebKitUserContentManager = struct _WebKitUserContentManager;
using WebKitUserScript = struct _WebKitUserScript;

enum WebKitUserContentInjectedFrames {
  WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
  WEBKIT_USER_CONTENT_INJECT_TOP_FRAME
};

enum WebKitUserScriptInjectionTime {
  WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
  WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_END,
};

using webkit_web_view_new_func = std::add_pointer<GtkWidget*()>::type;
using webkit_web_view_get_settings_func = std::add_pointer<WebKitSettings*(WebKitWebView*)>::type;
using webkit_settings_set_javascript_can_access_clipboard_func = std::add_pointer<void(WebKitSettings*, bool)>::type;
using webkit_settings_set_enable_write_console_messages_to_stdout_func = std::add_pointer<void(WebKitSettings*, bool)>::type;
using webkit_settings_set_enable_developer_extras_func = std::add_pointer<void(WebKitSettings*, bool)>::type;
using webkit_web_view_get_inspector_func = std::add_pointer<WebKitWebInspector*(WebKitWebView*)>::type;
using webkit_web_inspector_show_func = std::add_pointer<void(WebKitWebInspector*)>::type;
using webkit_web_view_set_background_color_func = std::add_pointer<void(WebKitWebView*, const GdkRGBA*)>::type;
using webkit_web_view_get_user_content_manager_func = std::add_pointer<WebKitUserContentManager*(WebKitWebView*)>::type;
using webkit_user_content_manager_add_script_func = std::add_pointer<void(WebKitUserContentManager*, WebKitUserScript*)>::type;
using webkit_user_script_new_func = std::add_pointer<WebKitUserScript*(const char*, WebKitUserContentInjectedFrames, WebKitUserScriptInjectionTime, const char*, const char*)>::type;
using webkit_settings_get_user_agent_func = std::add_pointer<const char*(WebKitSettings*)>::type;
using webkit_settings_set_user_agent_func = std::add_pointer<void(WebKitSettings*, const char*)>::type;
using webkit_web_view_load_uri_func = std::add_pointer<void(WebKitWebView*, const char*)>::type;

namespace webview {

static webkit_web_view_new_func webkit_web_view_new = nullptr;
static webkit_web_view_get_settings_func webkit_web_view_get_settings = nullptr;
static webkit_settings_set_javascript_can_access_clipboard_func webkit_settings_set_javascript_can_access_clipboard = nullptr;
static webkit_settings_set_enable_write_console_messages_to_stdout_func webkit_settings_set_enable_write_console_messages_to_stdout = nullptr;
static webkit_settings_set_enable_developer_extras_func webkit_settings_set_enable_developer_extras = nullptr;
static webkit_web_view_get_inspector_func webkit_web_view_get_inspector = nullptr;
static webkit_web_inspector_show_func webkit_web_inspector_show = nullptr;
static webkit_web_view_set_background_color_func webkit_web_view_set_background_color = nullptr;
static webkit_web_view_get_user_content_manager_func webkit_web_view_get_user_content_manager = nullptr;
static webkit_user_content_manager_add_script_func webkit_user_content_manager_add_script = nullptr;
static webkit_user_script_new_func webkit_user_script_new = nullptr;
static webkit_settings_get_user_agent_func webkit_settings_get_user_agent = nullptr;
static webkit_settings_set_user_agent_func webkit_settings_set_user_agent = nullptr;
static webkit_web_view_load_uri_func webkit_web_view_load_uri = nullptr;

static bool gtkSupportsAlpha = true;
static void *dlib = nullptr;

class gtk_webkit_engine {
public:
  gtk_webkit_engine(bool debug, bool openInspector, void *window, bool transparent, const std::string &args)
      : m_window(static_cast<GtkWidget *>(window)) {
        
    setlocale(LC_ALL, "");

    XInitThreads();
    gtk_init_check(0, NULL);
    m_window = static_cast<GtkWidget *>(window);
    if (m_window == nullptr) {
      m_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    }

    if(transparent) {
      GdkScreen *screen = gtk_widget_get_screen(m_window);
      GdkVisual *visual = gdk_screen_get_rgba_visual(screen);

      if(!visual) {
      visual = gdk_screen_get_system_visual(screen);
      gtkSupportsAlpha = false;
      }

      gtk_widget_set_app_paintable(m_window, true);
      gtk_widget_set_visual(m_window, visual);

      g_signal_connect(G_OBJECT(m_window), "draw",
          G_CALLBACK(+[](GtkWidget *widget, cairo_t *cr, gpointer userdata) {

          if(gtkSupportsAlpha) {
          cairo_set_source_rgba(cr, 0, 0, 0, 0);
          }
          else {
          cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
          }

          cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
          cairo_paint(cr);

          return false;
      }),
      nullptr);
    }

    g_signal_connect(G_OBJECT(m_window), "destroy",
                     G_CALLBACK(+[](GtkWidget *, gpointer arg) {
                       std::exit(processExitCode);
                     }),
                     this);

    g_signal_connect(G_OBJECT(m_window), "delete-event",
                    G_CALLBACK(+[](GtkWidget *, gpointer arg) {
                        if(windowStateChange)
                            windowStateChange(WEBVIEW_WINDOW_CLOSE);
                        return true;
                    }),
                    nullptr);

    g_signal_connect(G_OBJECT(m_window), "window-state-event",
        G_CALLBACK(+[](GtkWidget *widget, GdkEventWindowState *event, gpointer user_data) {
            if(!windowStateChange) return;

            if(event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN) {
                windowStateChange(event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN ? 
                  WEBVIEW_WINDOW_FULLSCREEN : WEBVIEW_WINDOW_UNFULLSCREEN);
            }
            else if(event->changed_mask & GDK_WINDOW_STATE_ICONIFIED) {
                windowStateChange(event->new_window_state & GDK_WINDOW_STATE_ICONIFIED ? 
                  WEBVIEW_WINDOW_MINIMIZED : WEBVIEW_WINDOW_RESTORED);
            }
            else if(event->changed_mask & GDK_WINDOW_STATE_FOCUSED) {
                windowStateChange(event->new_window_state & GDK_WINDOW_STATE_FOCUSED ? 
                  WEBVIEW_WINDOW_FOCUS : WEBVIEW_WINDOW_BLUR);
            }
            else if(event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED && 
                event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) {
                windowStateChange(WEBVIEW_WINDOW_MAXIMIZE);
            }
            else
                windowStateChange(WEBVIEW_WINDOW_UNDEFINED);
        }),
    nullptr);

    // libwebkit2gtk loader
    const std::vector<std::string> libs = {
      "libwebkit2gtk-4.0.so.37",
      "libwebkit2gtk-4.1.so.0"
    };

    for(const auto &lib: libs) {
      dlib = dlopen(lib.c_str(), RTLD_LAZY);

      if(dlib) {
        break;
      }
    }

    if(!dlib) {
      initCode = 1;
      return;
    }

    webkit_web_view_new = (webkit_web_view_new_func)(dlsym(dlib, "webkit_web_view_new"));
    webkit_web_view_get_settings = (webkit_web_view_get_settings_func)(dlsym(dlib, "webkit_web_view_get_settings"));
    webkit_settings_set_javascript_can_access_clipboard = (webkit_settings_set_javascript_can_access_clipboard_func)(dlsym(dlib, "webkit_settings_set_javascript_can_access_clipboard"));
    webkit_settings_set_enable_write_console_messages_to_stdout = (webkit_settings_set_enable_write_console_messages_to_stdout_func)(dlsym(dlib, "webkit_settings_set_enable_write_console_messages_to_stdout"));
    webkit_settings_set_enable_developer_extras = (webkit_settings_set_enable_developer_extras_func)(dlsym(dlib, "webkit_settings_set_enable_developer_extras"));
    webkit_web_view_get_inspector = (webkit_web_view_get_inspector_func)(dlsym(dlib, "webkit_web_view_get_inspector"));
    webkit_web_inspector_show = (webkit_web_inspector_show_func)(dlsym(dlib, "webkit_web_inspector_show"));
    webkit_web_view_set_background_color = (webkit_web_view_set_background_color_func)(dlsym(dlib, "webkit_web_view_set_background_color"));
    webkit_web_view_get_user_content_manager = (webkit_web_view_get_user_content_manager_func)(dlsym(dlib, "webkit_web_view_get_user_content_manager"));
    webkit_user_content_manager_add_script = (webkit_user_content_manager_add_script_func)(dlsym(dlib, "webkit_user_content_manager_add_script"));
    webkit_user_script_new = (webkit_user_script_new_func)(dlsym(dlib, "webkit_user_script_new"));
    webkit_settings_get_user_agent = (webkit_settings_get_user_agent_func)(dlsym(dlib, "webkit_settings_get_user_agent"));
    webkit_settings_set_user_agent = (webkit_settings_set_user_agent_func)(dlsym(dlib, "webkit_settings_set_user_agent"));
    webkit_web_view_load_uri = (webkit_web_view_load_uri_func)(dlsym(dlib, "webkit_web_view_load_uri"));


    // Initialize webview widget
    m_webview = webkit_web_view_new();

    GtkWidget *parentContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(parentContainer), GTK_WIDGET(m_webview), true, true, 0);    
    gtk_container_add(GTK_CONTAINER(m_window), parentContainer);

    gtk_widget_grab_focus(GTK_WIDGET(m_webview));

    WebKitSettings *settings =
        webkit_web_view_get_settings((WebKitWebView*)(m_webview));
    webkit_settings_set_javascript_can_access_clipboard(settings, true);
    if (debug) {
      webkit_settings_set_enable_write_console_messages_to_stdout(settings,
                                                                  true);
      webkit_settings_set_enable_developer_extras(settings, true);
      if (openInspector) {
        WebKitWebInspector *inspector = webkit_web_view_get_inspector((WebKitWebView*)(m_webview));
        webkit_web_inspector_show((WebKitWebInspector*)(inspector));
      }
    }

    if(transparent) {
      GdkRGBA color { 0, 0, 0, 0 };
      webkit_web_view_set_background_color((WebKitWebView*)(m_webview), &color);
    }

    gtk_widget_show_all(m_window);
  }
  void *window() { return (void *)m_window; }
  void *wv() { return (void *)m_webview; }
  void run() { gtk_main(); }
  void terminate(int exitCode = 0) {
    processExitCode = exitCode;
    gtk_window_close(GTK_WINDOW(m_window));
    gtk_widget_destroy(m_window);
  }
  void dispatch(std::function<void()> f) {
    g_idle_add_full(G_PRIORITY_HIGH_IDLE, (GSourceFunc)([](void *f) -> int {
                      (*static_cast<dispatch_fn_t *>(f))();
                      return G_SOURCE_REMOVE;
                    }),
                    new std::function<void()>(f),
                    [](void *f) { delete static_cast<dispatch_fn_t *>(f); });
  }

  void *dl() {
    return dlib;
  }

  void init(const std::string js) {
    WebKitUserContentManager *manager =
      webkit_web_view_get_user_content_manager((WebKitWebView*)(m_webview));
      webkit_user_content_manager_add_script(
          manager, webkit_user_script_new(
                      js.c_str(), WEBKIT_USER_CONTENT_INJECT_TOP_FRAME,
                      WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START, NULL, NULL));
  }

  void set_title(const std::string title) {
    gtk_window_set_title(GTK_WINDOW(m_window), title.c_str());

  }

  void extend_user_agent(const std::string customAgent) {
    WebKitSettings *settings =
      webkit_web_view_get_settings((WebKitWebView*)(m_webview));
      std::string ua = std::string(webkit_settings_get_user_agent(settings)) + " " + customAgent;
      webkit_settings_set_user_agent(settings, ua.c_str());
  }

  std::string get_title() {
    std::string title(gtk_window_get_title(GTK_WINDOW(m_window)));
    return title;
  }

  void set_size(int width, int height, int minWidth, int minHeight,
  int maxWidth, int maxHeight, bool resizable) {
    if(minWidth != -1 || minHeight != -1 || maxWidth != -1 || maxHeight != -1) {
      GdkGeometry g;
      GdkWindowHints h;

      if((minWidth != -1 || minHeight != -1) && (maxWidth != -1 || maxHeight != -1))
        h = (GdkWindowHints)(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE);
      else if(maxWidth != -1 || maxHeight != -1)
        h = GDK_HINT_MAX_SIZE;
      else
        h = GDK_HINT_MIN_SIZE;

      g.min_width = minWidth;
      g.min_height = minHeight;
      g.max_width = maxWidth;
      g.max_height = maxHeight;
      gtk_window_set_geometry_hints(GTK_WINDOW(m_window), NULL, &g, h);
    }
    gtk_window_set_resizable(GTK_WINDOW(m_window), resizable);
    if(width != -1 || height != -1) {
      if(resizable)
        gtk_window_resize(GTK_WINDOW(m_window), width, height);
      else
        gtk_widget_set_size_request(m_window, width, height);
    }
  }

  void navigate(const std::string url) {
    webkit_web_view_load_uri((WebKitWebView*)(m_webview), url.c_str());
  }

protected:
  int initCode = 0;

private:
  GtkWidget *m_window;
  GtkWidget *m_webview;
};

using browser_engine = gtk_webkit_engine;

} // namespace webview

#elif defined(WEBVIEW_COCOA)

//
// ====================================================================
//
// This implementation uses Cocoa WKWebView backend on macOS. It is
// written using ObjC runtime and uses WKWebView class as a browser runtime.
// You should pass "-framework Webkit" flag to the compiler.
//
// ====================================================================
//

#include <CoreGraphics/CoreGraphics.h>
#include <objc/objc-runtime.h>

#define NSBackingStoreBuffered 2

#define NSWindowStyleMaskResizable 8
#define NSWindowStyleMaskMiniaturizable 4
#define NSWindowStyleMaskTitled 1
#define NSWindowStyleMaskClosable 2

#define NSApplicationActivationPolicyRegular 0
#define NSApplicationActivationPolicyAccessory 1

#define WKUserScriptInjectionTimeAtDocumentStart 0

namespace webview {

// Helpers to avoid too much typing
id operator"" _cls(const char *s, std::size_t) { return (id)objc_getClass(s); }
SEL operator"" _sel(const char *s, std::size_t) { return sel_registerName(s); }
id operator"" _str(const char *s, std::size_t) {
  return ((id(*)(id, SEL, const char *))objc_msgSend)(
      "NSString"_cls, "stringWithUTF8String:"_sel, s);
}

class cocoa_wkwebview_engine {
public:
  cocoa_wkwebview_engine(bool debug, bool openInspector, void *window, bool transparent, const std::string &args) {
    // Application
    id app = ((id(*)(id, SEL))objc_msgSend)("NSApplication"_cls,
                                            "sharedApplication"_sel);
    ((void (*)(id, SEL, long))objc_msgSend)(
        app, "setActivationPolicy:"_sel, NSApplicationActivationPolicyRegular);

    // Delegate
    auto cls =
        objc_allocateClassPair((Class) "NSResponder"_cls, "AppDelegate", 0);
    class_addProtocol(cls, objc_getProtocol("NSTouchBarProvider"));
    class_addMethod(cls, "applicationShouldTerminateAfterLastWindowClosed:"_sel,
                    (IMP)(+[](id, SEL, id) -> BOOL { return 0; }), "c@:@");
    class_addMethod(cls, "menuCallback:"_sel,
      (IMP)(+[](id, SEL, id sender) -> void { 
      WindowMenuItem *m = ((WindowMenuItem *(*)(id, SEL))objc_msgSend)(
          ((id (*)(id, SEL))objc_msgSend)(sender, "representedObject"_sel),
          "pointerValue"_sel);

      if (m && m->cb) {
        m->cb(m);
      }

    }), "v@:@");

    objc_registerClassPair(cls);

    auto delegate = ((id(*)(id, SEL))objc_msgSend)((id)cls, "new"_sel);
    objc_setAssociatedObject(delegate, "webview", (id)this,
                             OBJC_ASSOCIATION_ASSIGN);
    ((void (*)(id, SEL, id))objc_msgSend)(app, sel_registerName("setDelegate:"),
                                          delegate);

    // Main window
    if (window == nullptr) {
      if (transparent) {
        m_window = ((id(*)(id, SEL))objc_msgSend)("MacWindow"_cls, "alloc"_sel);
      } else {
        m_window = ((id(*)(id, SEL))objc_msgSend)("NSWindow"_cls, "alloc"_sel);
      }
      m_window =
          ((id(*)(id, SEL, CGRect, int, unsigned long, int))objc_msgSend)(
              m_window, "initWithContentRect:styleMask:backing:defer:"_sel,
              CGRectMake(0, 0, 0, 0), 0, NSBackingStoreBuffered, 0);
    } else {
      m_window = (id)window;
    }

    // Main window delegate
    auto wcls =
        objc_allocateClassPair((Class) "NSResponder"_cls, "WindowDelegate", 0);
    class_addMethod(wcls, "windowShouldClose:"_sel,
                    (IMP)(+[](id, SEL, id) -> BOOL {
                      if(windowStateChange)
                        windowStateChange(WEBVIEW_WINDOW_CLOSE);
                      return 0;
                     }), "c@:@");
    class_addMethod(wcls, "windowDidBecomeKey:"_sel,
                    (IMP)(+[](id, SEL, id) {
                        if(windowStateChange)
                          windowStateChange(WEBVIEW_WINDOW_FOCUS);
                    }), "c@:@");
    class_addMethod(wcls, "windowDidResignKey:"_sel,
                    (IMP)(+[](id, SEL, id) {
                        if(windowStateChange)
                          windowStateChange(WEBVIEW_WINDOW_BLUR);
                    }), "c@:@");
    class_addMethod(wcls, "windowDidMiniaturize:"_sel,
                    (IMP)(+[](id, SEL, id) {
                        if(windowStateChange)
                          windowStateChange(WEBVIEW_WINDOW_MINIMIZED);
                    }), "c@:@");
    class_addMethod(wcls, "windowDidDeminiaturize:"_sel,
                    (IMP)(+[](id, SEL, id) {
                        if(windowStateChange)
                          windowStateChange(WEBVIEW_WINDOW_RESTORED);
                    }), "c@:@");
    class_addMethod(wcls, "windowDidEnterFullScreen:"_sel,
                    (IMP)(+[](id, SEL, id) {
                        if(windowStateChange)
                          windowStateChange(WEBVIEW_WINDOW_FULLSCREEN);
                    }), "c@:@");
    class_addMethod(wcls, "windowDidExitFullScreen:"_sel,
                    (IMP)(+[](id, SEL, id) {
                        if(windowStateChange)
                          windowStateChange(WEBVIEW_WINDOW_UNFULLSCREEN);
                    }), "c@:@");
    class_addMethod(wcls, "windowDidEndLiveResize:"_sel,
                    (IMP)(+[](id, SEL, id notification) {
                        bool isZoomed = ((bool (*)(id, SEL))objc_msgSend)(
                            ((id (*)(id, SEL))objc_msgSend)((id) notification,
                            "object"_sel),
                          "isZoomed"_sel);
                        if(isZoomed && windowStateChange)
                          windowStateChange(WEBVIEW_WINDOW_MAXIMIZE);
                    }), "c@:@");

    objc_registerClassPair(wcls);

    auto wdelegate = ((id(*)(id, SEL))objc_msgSend)((id)wcls, "new"_sel);
    objc_setAssociatedObject(delegate, "webview", (id)this,
                             OBJC_ASSOCIATION_ASSIGN);
    ((void (*)(id, SEL, id))objc_msgSend)(m_window, sel_registerName("setDelegate:"),
                                          wdelegate);

    // Webview
    auto config =
        ((id(*)(id, SEL))objc_msgSend)("WKWebViewConfiguration"_cls, "new"_sel);
    m_manager =
        ((id(*)(id, SEL))objc_msgSend)(config, "userContentController"_sel);
    m_webview = ((id(*)(id, SEL))objc_msgSend)("WKWebView"_cls, "alloc"_sel);

    if (debug) {
      // Equivalent Obj-C:
      // [[config preferences] setValue:@YES forKey:@"developerExtrasEnabled"];
      ((id(*)(id, SEL, id, id))objc_msgSend)(
          ((id(*)(id, SEL))objc_msgSend)(config, "preferences"_sel),
          "setValue:forKey:"_sel,
          ((id(*)(id, SEL, BOOL))objc_msgSend)("NSNumber"_cls,
                                               "numberWithBool:"_sel, 1),
          "developerExtrasEnabled"_str);
    }

    // Equivalent Obj-C:
    // [[config preferences] setValue:@YES forKey:@"fullScreenEnabled"];
    ((id(*)(id, SEL, id, id))objc_msgSend)(
        ((id(*)(id, SEL))objc_msgSend)(config, "preferences"_sel),
        "setValue:forKey:"_sel,
        ((id(*)(id, SEL, BOOL))objc_msgSend)("NSNumber"_cls,
                                             "numberWithBool:"_sel, 1),
        "fullScreenEnabled"_str);

    // Equivalent Obj-C:
    // [[config preferences] setValue:@YES forKey:@"javaScriptCanAccessClipboard"];
    ((id(*)(id, SEL, id, id))objc_msgSend)(
        ((id(*)(id, SEL))objc_msgSend)(config, "preferences"_sel),
        "setValue:forKey:"_sel,
        ((id(*)(id, SEL, BOOL))objc_msgSend)("NSNumber"_cls,
                                             "numberWithBool:"_sel, 1),
        "javaScriptCanAccessClipboard"_str);

    // Equivalent Obj-C:
    // [[config preferences] setValue:@YES forKey:@"DOMPasteAllowed"];
    ((id(*)(id, SEL, id, id))objc_msgSend)(
        ((id(*)(id, SEL))objc_msgSend)(config, "preferences"_sel),
        "setValue:forKey:"_sel,
        ((id(*)(id, SEL, BOOL))objc_msgSend)("NSNumber"_cls,
                                             "numberWithBool:"_sel, 1),
        "DOMPasteAllowed"_str);

    ((void (*)(id, SEL, CGRect, id))objc_msgSend)(
        m_webview, "initWithFrame:configuration:"_sel, CGRectMake(0, 0, 0, 0),
        config);
    ((void (*)(id, SEL, id, id))objc_msgSend)(
        m_manager, "addScriptMessageHandler:name:"_sel, delegate,
        "external"_str);

    if(transparent) {
      ((id (*)(id, SEL, id, id))objc_msgSend)((id) m_webview, "setValue:forKey:"_sel,
          ((id(*)(id, SEL, BOOL))objc_msgSend)("NSNumber"_cls, "numberWithBool:"_sel, 0),
          "drawsBackground"_str);
    }

    ((void (*)(id, SEL, id))objc_msgSend)(m_window, "setContentView:"_sel,
                                          m_webview);
    ((void (*)(id, SEL, id))objc_msgSend)(m_window, "makeKeyAndOrderFront:"_sel,
                                          nullptr);
  }
  ~cocoa_wkwebview_engine() { close(); }
  void *window() { return (void *)m_window; }
  void *wv() { return (void *)m_webview; }
  void terminate(int exitCode = 0) {
    close();
    ((void (*)(id, SEL, id))objc_msgSend)("NSApp"_cls, "terminate:"_sel,
                                          nullptr);
    std::exit(exitCode);
  }
  void run() {
    id app = ((id(*)(id, SEL))objc_msgSend)("NSApplication"_cls,
                                            "sharedApplication"_sel);
    dispatch([&]() {
      ((void (*)(id, SEL, BOOL))objc_msgSend)(
          app, "activateIgnoringOtherApps:"_sel, 1);
    });
    ((void (*)(id, SEL))objc_msgSend)(app, "run"_sel);
  }
  void dispatch(std::function<void()> f) {
    dispatch_async_f(dispatch_get_main_queue(), new dispatch_fn_t(f),
                     (dispatch_function_t)([](void *arg) {
                       auto f = static_cast<dispatch_fn_t *>(arg);
                       (*f)();
                       delete f;
                     }));
  }

  void init(const std::string js) {
    // Equivalent Obj-C:
    // [m_manager addUserScript:[[WKUserScript alloc] initWithSource:[NSString stringWithUTF8String:js.c_str()] injectionTime:WKUserScriptInjectionTimeAtDocumentStart forMainFrameOnly:YES]]
    ((void (*)(id, SEL, id))objc_msgSend)(
        m_manager, "addUserScript:"_sel,
        ((id(*)(id, SEL, id, long, BOOL))objc_msgSend)(
            ((id(*)(id, SEL))objc_msgSend)("WKUserScript"_cls, "alloc"_sel),
            "initWithSource:injectionTime:forMainFrameOnly:"_sel,
            ((id(*)(id, SEL, const char *))objc_msgSend)(
                "NSString"_cls, "stringWithUTF8String:"_sel, js.c_str()),
            WKUserScriptInjectionTimeAtDocumentStart, 1));
  }

  void extend_user_agent(const std::string customAgent) {
    std::string ua = std::string(
      ((const char *(*)(id, SEL))objc_msgSend)(
        ((id(*)(id, SEL, id))objc_msgSend)(m_webview, "valueForKey:"_sel,
        "userAgent"_str), "UTF8String"_sel)
    );
    std::string newUa = ua + " " + customAgent;
    ((id(*)(id, SEL, id, id))objc_msgSend)(
        m_webview,
        "setValue:forKey:"_sel,
        ((id(*)(id, SEL, const char *))objc_msgSend)(
            "NSString"_cls, "stringWithUTF8String:"_sel, newUa.c_str()),
        "customUserAgent"_str);
  }

  void set_title(const std::string title) {
    ((void (*)(id, SEL, id))objc_msgSend)(
        m_window, "setTitle:"_sel,
        ((id(*)(id, SEL, const char *))objc_msgSend)(
            "NSString"_cls, "stringWithUTF8String:"_sel, title.c_str()));
  }

  std::string get_title() {
    std::string title = std::string(
      ((const char *(*)(id, SEL))objc_msgSend)(
        ((id(*)(id, SEL))objc_msgSend)(m_window, "title"_sel)
      , "UTF8String"_sel)
    );
    return title;
  }

  void set_size(int width, int height, int minWidth, int minHeight,
                int maxWidth, int maxHeight, bool resizable) {
    auto style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                 NSWindowStyleMaskMiniaturizable;
    if (resizable) {
      style = style | NSWindowStyleMaskResizable;
    }
    ((void (*)(id, SEL, unsigned long))objc_msgSend)(
        m_window, "setStyleMask:"_sel, style);

    if (minWidth != -1 || minHeight != -1) {
      ((void (*)(id, SEL, CGSize))objc_msgSend)(
          m_window, "setContentMinSize:"_sel, CGSizeMake(minWidth, minHeight));
    }
    if (maxWidth != -1 || maxHeight != -1) {
      ((void (*)(id, SEL, CGSize))objc_msgSend)(
          m_window, "setContentMaxSize:"_sel, CGSizeMake(maxWidth, maxHeight));
    }
    if(width != -1 || height != -1) {
      ((void (*)(id, SEL, CGRect, BOOL, BOOL))objc_msgSend)(
          m_window, "setFrame:display:animate:"_sel,
          CGRectMake(0, 0, width, height), 1, 0);
      ((void (*)(id, SEL))objc_msgSend)(m_window, "center"_sel);
    }
  }
  void navigate(const std::string url) {
    auto nsurl = ((id(*)(id, SEL, id))objc_msgSend)(
        "NSURL"_cls, "URLWithString:"_sel,
        ((id(*)(id, SEL, const char *))objc_msgSend)(
            "NSString"_cls, "stringWithUTF8String:"_sel, url.c_str()));

    ((void (*)(id, SEL, id))objc_msgSend)(
        m_webview, "loadRequest:"_sel,
        ((id(*)(id, SEL, id))objc_msgSend)("NSURLRequest"_cls,
                                           "requestWithURL:"_sel, nsurl));
  }


protected:
  int initCode = 0;

private:
  void close() { ((void (*)(id, SEL))objc_msgSend)(m_window, "close"_sel); }
  id m_window;
  id m_webview;
  id m_manager;
};

using browser_engine = cocoa_wkwebview_engine;

} // namespace webview

#elif defined(WEBVIEW_EDGE)

//
// ====================================================================
//
// This implementation uses Win32 API to create a native window. It can
// use either EdgeHTML or Edge/Chromium backend as a browser engine.
//
// ====================================================================
//

#define WIN32_LEAN_AND_MEAN
#include <Shlwapi.h>
#include <codecvt>
#include <stdlib.h>
#include <windows.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Shlwapi.lib")

// Edge/Chromium headers and libs
#include "webview2.h"
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

// Tray lib types
#define TRAY_WINAPI 1
#include "lib/tray/tray.h"

#include "darkmode.h"

#define WM_WINDOW_PASS_MENU_REFS (WM_USER + 3)
#define WM_WINDOW_DELETE_MENU_REFS (WM_USER + 4)
#define ID_MENU_FIRST 20000

namespace webview {

class edge_chromium {
public:
  bool embed(HWND wnd, bool debug, bool openInspector) {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    flag.test_and_set();

    // Use Unicode APIs to properly handle special characters in paths
    wchar_t currentExePath[MAX_PATH];
    GetModuleFileNameW(NULL, currentExePath, MAX_PATH);
    wchar_t *currentExeName = PathFindFileNameW(currentExePath);

    // Get APPDATA with proper null check and fallback
    wchar_t* appdataEnv = _wgetenv(L"APPDATA");
    std::wstring userDataFolder;
    if (appdataEnv) {
        userDataFolder = std::wstring(appdataEnv);
    } else {
        // Fallback to temp directory if APPDATA is not available
        wchar_t tempPath[MAX_PATH];
        GetTempPathW(MAX_PATH, tempPath);
        userDataFolder = std::wstring(tempPath);
    }
    std::wstring currentExeNameW = currentExeName ? std::wstring(currentExeName) : L"neutralino";

    // Ensure the user data folder path is properly formatted
    if (!userDataFolder.empty() && userDataFolder.back() != L'\\' && userDataFolder.back() != L'/') {
        userDataFolder += L"\\";
    }

    HRESULT res = CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        (userDataFolder + currentExeNameW).c_str(),
        nullptr,
        new webview2_com_handler(wnd, [&](ICoreWebView2Controller* controller) {
            m_controller = controller;
            m_controller->get_CoreWebView2(&m_webview);
            m_webview->AddRef();

            ICoreWebView2Settings* m_settings;
            m_webview->get_Settings(&m_settings);
            if (debug) {
                m_settings->put_AreDevToolsEnabled(TRUE);
                if (openInspector)
                  m_webview->OpenDevToolsWindow();
            }
            else {
                m_settings->put_AreDevToolsEnabled(FALSE);
                m_settings->put_IsStatusBarEnabled(FALSE);
            }
            flag.clear();
        }
    ));
    if (res != S_OK) {
      CoUninitialize();
      return false;
    }
    MSG msg = {};
    while (flag.test_and_set() && GetMessage(&msg, NULL, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    return true;
  }

  void init(const std::string js) {
    LPCWSTR wjs = to_lpwstr(js);
    m_webview->AddScriptToExecuteOnDocumentCreated(wjs, nullptr);
    delete[] wjs;
  }

  void extend_user_agent(const std::string customAgent) {
    ICoreWebView2Settings *settings = nullptr;
    m_webview->get_Settings(&settings);
    ICoreWebView2Settings2 *settings2 = nullptr;
    settings->QueryInterface(IID_ICoreWebView2Settings2, reinterpret_cast<void**>(&settings2));
    LPWSTR ua;
    settings2->get_UserAgent(&ua);
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> wideCharConverter;
    std::string newUa = wideCharConverter.to_bytes(ua) + " " + customAgent;
    settings2->put_UserAgent(to_lpwstr(newUa));
    settings2->Release();
    CoTaskMemFree(ua);
  }

  void resize(HWND wnd) {
    if (m_controller == nullptr) {
      return;
    }
    RECT bounds;
    GetClientRect(wnd, &bounds);
    m_controller->put_Bounds(bounds);
  }

  void navigate(const std::string url) {
    auto wurl = to_lpwstr(url);
    m_webview->Navigate(wurl);
    delete[] wurl;
  }

  void *wv() {
    return (void *)m_webview;
  }

private:
  LPWSTR to_lpwstr(const std::string s) {
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    wchar_t *ws = new wchar_t[n];
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, ws, n);
    return ws;
  }

  ICoreWebView2 *m_webview = nullptr;
  ICoreWebView2Controller *m_controller = nullptr;

  class webview2_com_handler
      : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler,
        public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler,
        public ICoreWebView2WebMessageReceivedEventHandler,
        public ICoreWebView2PermissionRequestedEventHandler {
    using webview2_com_handler_cb_t =
        std::function<void(ICoreWebView2Controller *)>;

  public:
    webview2_com_handler(HWND hwnd,
                         webview2_com_handler_cb_t cb)
        : m_window(hwnd), m_cb(cb) {}
    ULONG STDMETHODCALLTYPE AddRef() { return 1; }
    ULONG STDMETHODCALLTYPE Release() { return 1; }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppv) {
      return S_OK;
    }
    HRESULT STDMETHODCALLTYPE Invoke(HRESULT res,
                                     ICoreWebView2Environment *env) {
      env->CreateCoreWebView2Controller(m_window, this);
      return S_OK;
    }
    HRESULT STDMETHODCALLTYPE Invoke(HRESULT res,
                                     ICoreWebView2Controller *controller) {
      controller->AddRef();

      ICoreWebView2 *webview;
      ::EventRegistrationToken token;
      controller->get_CoreWebView2(&webview);
      webview->add_WebMessageReceived(this, &token);
      webview->add_PermissionRequested(this, &token);

      m_cb(controller);
      return S_OK;
    }
    HRESULT STDMETHODCALLTYPE Invoke(
        ICoreWebView2 *sender, ICoreWebView2WebMessageReceivedEventArgs *args) {
      LPWSTR message;
      args->TryGetWebMessageAsString(&message);

      std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> wideCharConverter;
      sender->PostWebMessageAsString(message);

      CoTaskMemFree(message);
      return S_OK;
    }
    HRESULT STDMETHODCALLTYPE
    Invoke(ICoreWebView2 *sender,
           ICoreWebView2PermissionRequestedEventArgs *args) {
      COREWEBVIEW2_PERMISSION_KIND kind;
      args->get_PermissionKind(&kind);
      if (kind == COREWEBVIEW2_PERMISSION_KIND_CLIPBOARD_READ) {
        args->put_State(COREWEBVIEW2_PERMISSION_STATE_ALLOW);
      }
      return S_OK;
    }

  private:
    HWND m_window;
    webview2_com_handler_cb_t m_cb;
  };
};

class win32_edge_engine {
public:
  win32_edge_engine(bool debug, bool openInspector, void *window, bool transparent, const std::string& args) {
    if(args != "") {
        std::wstring wargs = str2wstr(args);
        SetEnvironmentVariableW(L"WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS", wargs.c_str());
    }
    if (window == nullptr) {
      HINSTANCE hInstance = GetModuleHandle(nullptr);
      HICON icon = (HICON)LoadImage(
          hInstance, IDI_APPLICATION, IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),
          GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
      static UINT WM_TASKBAR_CREATED = RegisterWindowMessage(L"TaskbarCreated");
      WNDCLASSEX wc;
      ZeroMemory(&wc, sizeof(WNDCLASSEX));
      wc.cbSize = sizeof(WNDCLASSEX);
      wc.hInstance = hInstance;
      wc.lpszClassName = L"Neutralinojs_webview";
      wc.hIcon = icon;
      wc.hIconSm = icon;
      wc.lpfnWndProc =
          (WNDPROC)(+[](HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) -> int {
            auto w = (win32_edge_engine *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            static HMENU menuRef;
            static std::vector<HMENU> menuRefs;
            switch (msg) {
            case WM_SIZE:
              w->m_browser->resize(hwnd);
              if(!windowStateChange) break;
              if(wp == SIZE_MINIMIZED) 
                windowStateChange(WEBVIEW_WINDOW_MINIMIZED);
              else if(wp == SIZE_RESTORED) 
                windowStateChange(WEBVIEW_WINDOW_RESTORED);
              else if(wp == SIZE_MAXIMIZED) 
                windowStateChange(WEBVIEW_WINDOW_MAXIMIZE);
              break;
            case WM_CLOSE:
              if(windowStateChange)
                windowStateChange(WEBVIEW_WINDOW_CLOSE);
              break;
            case WM_ACTIVATE:
              if(!windowStateChange) break;
              if(LOWORD(wp) == WA_INACTIVE)
                windowStateChange(WEBVIEW_WINDOW_BLUR);
              else
                windowStateChange(WEBVIEW_WINDOW_FOCUS);
              break;
            case WM_DESTROY:
              PostQuitMessage(processExitCode);
              break;
            case WM_QUIT:
              ExitProcess(wp);
              break;
            // ---- Begin Tray lib related --------
            case WM_TRAY_PASS_MENU_REF:
              menuRef = (HMENU) wp;
              break;
            case WM_WINDOW_PASS_MENU_REFS:
              menuRefs.push_back((HMENU) wp);
              break;
            case WM_WINDOW_DELETE_MENU_REFS:
              menuRefs.clear();
              break;
            case WM_TRAY_CALLBACK_MESSAGE:
              if (lp == WM_LBUTTONUP || lp == WM_RBUTTONUP) {
                POINT p;
                GetCursorPos(&p);
                SetForegroundWindow(hwnd);
                WORD cmd = TrackPopupMenu(menuRef, TPM_LEFTALIGN | TPM_RIGHTBUTTON |
                                                    TPM_RETURNCMD | TPM_NONOTIFY,
                                          p.x, p.y, 0, hwnd, nullptr);
                SendMessage(hwnd, WM_COMMAND, cmd, 0);
              }
              break;
            case WM_COMMAND:
              if (wp >= ID_MENU_FIRST) {
                for(const HMENU itMenuRef: menuRefs) {
                  MENUITEMINFO item;
                  memset(&item, 0, sizeof(item));
                  item.cbSize = sizeof(MENUITEMINFO);
                  item.fMask = MIIM_ID | MIIM_DATA;
                  if (GetMenuItemInfo(itMenuRef, wp, false, &item)) {
                    WindowMenuItem *menu = (WindowMenuItem *)item.dwItemData;
                    if (menu != nullptr && menu->cb != nullptr) {
                      menu->cb(menu);
                    }
                    break;
                  }
                }
                return 0;
              }
              else if (wp >= ID_TRAY_FIRST) {
                MENUITEMINFO item;
                memset(&item, 0, sizeof(item));
                item.cbSize = sizeof(MENUITEMINFO);
                item.fMask = MIIM_ID | MIIM_DATA;
                if (GetMenuItemInfo(menuRef, wp, false, &item)) {
                  struct tray_menu *menu = (struct tray_menu *)item.dwItemData;
                  if (menu != nullptr && menu->cb != nullptr) {
                    menu->cb(menu);
                  }
                }
                return 0;
              }
              break;
            // ---- /End Tray lib related --------
            case WM_GETMINMAXINFO: {
              auto lpmmi = (LPMINMAXINFO)lp;
              if (w == nullptr) {
                return 0;
              }
              if (w->m_maxsz.x > 0 && w->m_maxsz.y > 0) {
                lpmmi->ptMaxSize = w->m_maxsz;
                lpmmi->ptMaxTrackSize = w->m_maxsz;
              }
              if (w->m_minsz.x > 0 && w->m_minsz.y > 0) {
                lpmmi->ptMinTrackSize = w->m_minsz;
              }
            } break;
            default:
              if (msg == WM_TASKBAR_CREATED) {
                tray_recreate();
              }
              return DefWindowProc(hwnd, msg, wp, lp);
            }
            return 0;
          });
      RegisterClassEx(&wc);
      int width = transparent ? 8000 : 640;
      int height = transparent ? 8000 : 480;
      m_window = CreateWindow(L"Neutralinojs_webview", L"", WS_OVERLAPPEDWINDOW, 99999999,
                              CW_USEDEFAULT, width, height, nullptr, nullptr,
                              GetModuleHandle(nullptr), nullptr);
      SetWindowLongPtr(m_window, GWLP_USERDATA, (LONG_PTR)this);
    } else {
      m_window = *(static_cast<HWND *>(window));
    }

    setDpi();

    if (transparent) {
      SetWindowLong(m_window, GWL_EXSTYLE, GetWindowLong(m_window, GWL_EXSTYLE) | WS_EX_LAYERED);
      // transparent white, use of environment variable prevents flashing on show
      SetEnvironmentVariable(L"WEBVIEW2_DEFAULT_BACKGROUND_COLOR", L"00FFFFFF");
    }

    // stop the taskbar icon from showing by removing WS_EX_APPWINDOW.
    SetWindowLong(m_window, GWL_EXSTYLE, GetWindowLong(m_window, GWL_EXSTYLE) & ~WS_EX_APPWINDOW);
    ShowWindow(m_window, SW_SHOW);
    UpdateWindow(m_window);
    SetForegroundWindow(m_window);

    // store the original initial window style
    m_originalStyleEx = GetWindowLong(m_window, GWL_EXSTYLE);

    // set dark mode of title bar according to system theme
    TrySetWindowTheme(m_window);

    if (!m_browser->embed(m_window, debug, openInspector)) {
      initCode = 1;
    }

    m_browser->resize(m_window);
  }

  void run() {
    MSG msg;
    BOOL res;
    while ((res = GetMessage(&msg, nullptr, 0, 0)) != -1) {
      if (msg.hwnd) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        continue;
      }
      if (msg.message == WM_APP) {
        auto f = (dispatch_fn_t *)(msg.lParam);
        (*f)();
        delete f;
      } else if (msg.message == WM_QUIT) {
        ExitProcess(msg.wParam);
        return;
      }
    }
  }
  void *window() { return (void *)m_window; }
  void *wv() { return (void *)m_browser->wv(); }
  void terminate(int exitCode = 0) {

    // event to wait for window close completion
    auto evtWindowClosed = CreateEvent(
        NULL,               // default security attributes
        TRUE,               // manual-reset event
        FALSE,              // initial state is nonsignaled
        TEXT("WindowClosedEvent")  // object name
    );
    processExitCode = exitCode;

    dispatch([=]() {
        DestroyWindow(m_window);
        SetEvent(evtWindowClosed);
    });

    // wait for dispatch() to complete
    /* TODO: Check why this method doesn't trigger a signal on Windows
    *        when "exitProcessOnClose" is true. (Previous value: 10000)
    *        This wait causes a delay when closing the program.
    */
    WaitForSingleObject(evtWindowClosed, 300);
    CloseHandle(evtWindowClosed);

  }
  void dispatch(dispatch_fn_t f) {
    PostThreadMessage(m_main_thread, WM_APP, 0, (LPARAM) new dispatch_fn_t(f));
  }

  void set_title(const std::string title) {
    SetWindowText(m_window, str2wstr(title).c_str());
  }

  std::string get_title() {
      int len = GetWindowTextLength(m_window);
      if (len == 0) return "";
      std::wstring title(len + 1, 0);
      if (!GetWindowText(m_window, &title[0], len + 1)) return "";
      title.resize(len);
      return wstr2str(title);
  }

  void set_size(int width, int height, int minWidth, int minHeight,
                int maxWidth, int maxHeight, bool resizable) {
    auto style = GetWindowLong(m_window, GWL_STYLE);
    if (!resizable) {
      style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    } else {
      style |= (WS_THICKFRAME | WS_MAXIMIZEBOX);
    }
    SetWindowLong(m_window, GWL_STYLE, style);

    if (maxWidth != -1 || maxHeight != -1) {
      m_maxsz.x = maxWidth;
      m_maxsz.y = maxHeight;
    }
    if (minWidth != -1 || minHeight != -1) {
      m_minsz.x = minWidth;
      m_minsz.y = minHeight;
    }
    if(width != -1 || height != -1) {
      RECT r;
      r.left = r.top = 0;
      r.right = width;
      r.bottom = height;
      SetWindowPos(
          m_window, NULL, r.left, r.top, r.right - r.left, r.bottom - r.top,
          SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_FRAMECHANGED);
      m_browser->resize(m_window);
    }
  }

  void navigate(const std::string url) { m_browser->navigate(url); }
  void init(const std::string js) { m_browser->init(js); }
  void extend_user_agent(const std::string customAgent) { m_browser->extend_user_agent(customAgent); }

  DWORD m_originalStyleEx;

protected:
  int initCode = 0;

private:

  void setDpi() {
    HMODULE user32 = LoadLibraryA("User32.dll");
    auto func_win10 = reinterpret_cast<decltype(&SetProcessDpiAwarenessContext)>(
      GetProcAddress(user32, "SetProcessDpiAwarenessContext"));
    if (func_win10) {
        // Windows 10+
        func_win10(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
    }
  }

  HWND m_window;
  POINT m_minsz = POINT{0, 0};
  POINT m_maxsz = POINT{0, 0};
  DWORD m_main_thread = GetCurrentThreadId();
  std::unique_ptr<webview::edge_chromium> m_browser =
      std::make_unique<webview::edge_chromium>();

  static std::wstring str2wstr(std::string const &str) {
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    std::wstring ret(len, '\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), (LPWSTR)ret.data(), (int)ret.size());
    return ret;
  }

  std::string wstr2str(std::wstring const &str)
  {
    int len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0, nullptr, nullptr);
    std::string ret(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(), (LPSTR)ret.data(), (int)ret.size(), nullptr, nullptr);
    return ret;
  }
};

using browser_engine = win32_edge_engine;
} // namespace webview

#endif /* WEBVIEW_GTK, WEBVIEW_COCOA, WEBVIEW_EDGE */

namespace webview {

class webview : public browser_engine {
public:
  webview(bool debug = false, bool openInspector = true, void *wnd = nullptr, bool transparent = false, const std::string& args = "")
      : browser_engine(debug, openInspector, wnd, transparent, args) {}

  void navigate(const std::string url) {
    browser_engine::navigate(url);
  }

  void setEventHandler(eventHandler_t handler) {
    windowStateChange = handler;
  }

  int get_init_code() {
    return initCode;
  }

};
} // namespace webview


#endif /* WEBVIEW_HEADER */

#endif /* WEBVIEW_H */
