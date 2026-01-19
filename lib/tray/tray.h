#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRAY_H
#define TRAY_H

#if defined(__APPLE__)
#include <objc/objc-runtime.h>
#include <limits.h>
#endif

struct tray_menu;

struct tray {
  #if defined(__linux__) || defined(__FreeBSD__)
  const char *icon = NULL;
  #elif defined(__APPLE__)
  id icon = NULL;
  #elif defined(_WIN32)
  HICON icon = NULL;
  #endif
  struct tray_menu *menu;
};

struct tray_menu {
  const char *id;
  const char *text;
  int disabled;
  int checked;

  void (*cb)(struct tray_menu *);
  void *context;

  struct tray_menu *submenu;
};

static void tray_update(struct tray *tray);

#if defined(TRAY_APPINDICATOR)

#include <gtk/gtk.h>
#include <dlfcn.h>

#define TRAY_APPINDICATOR_ID "tray-id"

typedef enum {
  APP_INDICATOR_CATEGORY_APPLICATION_STATUS,
  APP_INDICATOR_CATEGORY_COMMUNICATIONS,
  APP_INDICATOR_CATEGORY_SYSTEM_SERVICES,
  APP_INDICATOR_CATEGORY_HARDWARE,
  APP_INDICATOR_CATEGORY_OTHER
} AppIndicatorCategory;

typedef enum {
  APP_INDICATOR_STATUS_PASSIVE,
  APP_INDICATOR_STATUS_ACTIVE,
  APP_INDICATOR_STATUS_ATTENTION
} AppIndicatorStatus;

typedef struct _AppIndicator AppIndicator;

typedef AppIndicator* (*app_indicator_new_func)(const gchar *id,
                                                const gchar *icon_name,
                                                AppIndicatorCategory category);
typedef void (*app_indicator_set_status_func)(AppIndicator *self, AppIndicatorStatus status);
typedef void (*app_indicator_set_menu_func)(AppIndicator *self, GtkMenu *menu);
typedef void (*app_indicator_set_icon_func)(AppIndicator *self, const gchar *icon_name);

app_indicator_new_func app_indicator_new = NULL;
app_indicator_set_status_func app_indicator_set_status = NULL;
app_indicator_set_menu_func app_indicator_set_menu = NULL;
app_indicator_set_icon_func app_indicator_set_icon = NULL;

static AppIndicator *indicator = NULL;
static int loop_result = 0;

int load_app_indicator() {
  const char *dlib_names[] = {
    "libappindicator3.so",
    "libappindicator3.so.1",
    "libayatana-appindicator3.so",
    "libayatana-appindicator3.so.1"
  };

  void *dlib = NULL;

  for(int i = 0; i < sizeof(dlib_names) / sizeof(dlib_names[0]); i++) {
    dlib = dlopen(dlib_names[i], RTLD_LAZY);

    if(dlib) {
      break;
    }
  }

  if(!dlib) {
    return -1;
  }

  app_indicator_new = (app_indicator_new_func)(dlsym(dlib, "app_indicator_new"));
  app_indicator_set_status = (app_indicator_set_status_func)(dlsym(dlib, "app_indicator_set_status"));
  app_indicator_set_menu = (app_indicator_set_menu_func)(dlsym(dlib, "app_indicator_set_menu"));
  app_indicator_set_icon = (app_indicator_set_icon_func)(dlsym(dlib, "app_indicator_set_icon"));

  return 0;
}

static void _tray_menu_cb(GtkMenuItem *item, gpointer data) {
  (void)item;
  struct tray_menu *m = (struct tray_menu *)data;
  m->cb(m);
}

static GtkMenuShell *_tray_menu(struct tray_menu *m) {
  GtkMenuShell *menu = (GtkMenuShell *)gtk_menu_new();
  for (; m != NULL && m->text != NULL; m++) {
    GtkWidget *item;
    if (strcmp(m->text, "-") == 0) {
      item = gtk_separator_menu_item_new();
    } else {
      if (m->submenu != NULL) {
        item = gtk_menu_item_new_with_label(m->text);
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(item),
                                  GTK_WIDGET(_tray_menu(m->submenu)));
      } else if(m->checked) {
        item = gtk_check_menu_item_new_with_label(m->text);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), !!m->checked);
      } else {
        item = gtk_menu_item_new_with_label(m->text);
      }
      gtk_widget_set_sensitive(item, !m->disabled);
      if (m->cb != NULL) {
        g_signal_connect(item, "activate", G_CALLBACK(_tray_menu_cb), m);
      }
    }
    gtk_widget_show(item);
    gtk_menu_shell_append(menu, item);
  }
  return menu;
}

static int tray_init(struct tray *tray) {
  if (gtk_init_check(0, NULL) == FALSE) {
    return -1;
  }

  if(load_app_indicator() != 0) {
    return -1;
  }

  indicator = app_indicator_new(TRAY_APPINDICATOR_ID, tray->icon,
                                APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
  app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
  tray_update(tray);
  return 0;
}

static int tray_loop(int blocking) {
  gtk_main_iteration_do(blocking);
  return loop_result;
}

static void tray_update(struct tray *tray) {
  app_indicator_set_icon(indicator, tray->icon);
  // GTK is all about reference counting, so previous menu should be destroyed
  // here
  app_indicator_set_menu(indicator, GTK_MENU(_tray_menu(tray->menu)));
}

static void tray_exit() { loop_result = -1; }

#elif defined(TRAY_APPKIT)

static id app;
static id pool;
static id statusBar;
static id statusItem;
static id statusBarButton;

static id _tray_menu(struct tray_menu *m) {
  id menu = ((id (*)(id, SEL))objc_msgSend)(
          (id)objc_getClass("NSMenu"),
          sel_registerName("new"));

  ((id (*)(id, SEL))objc_msgSend)(menu, sel_registerName("autorelease"));
  ((id (*)(id, SEL, bool))objc_msgSend)(menu, sel_registerName("setAutoenablesItems:"), false);

  for (; m != NULL && m->text != NULL; m++) {
    if (strcmp(m->text, "-") == 0) {
      id separatorItem = ((id (*)(id, SEL))objc_msgSend)(
              (id)objc_getClass("NSMenuItem"),
              sel_registerName("separatorItem"));

      ((id (*)(id, SEL, id))objc_msgSend)(
          menu,
          sel_registerName("addItem:"),
          separatorItem);
    } else {
      id menuItem = ((id (*)(id, SEL))objc_msgSend)(
              (id)objc_getClass("NSMenuItem"),
              sel_registerName("alloc"));

      ((id (*)(id, SEL))objc_msgSend)(menuItem, sel_registerName("autorelease"));

      ((id (*)(id, SEL, id, SEL, id))objc_msgSend)(
          menuItem,
          sel_registerName("initWithTitle:action:keyEquivalent:"),
          ((id (*)(id, SEL, const char *))objc_msgSend)((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), m->text),
          sel_registerName("menuCallback:"),
          ((id (*)(id, SEL, const char *))objc_msgSend)((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), ""));

      ((id (*)(id, SEL, bool))objc_msgSend)(menuItem, sel_registerName("setEnabled:"), (m->disabled ? false : true));
      ((id (*)(id, SEL, bool))objc_msgSend)(menuItem, sel_registerName("setState:"), (m->checked ? 1 : 0));

      ((id (*)(id, SEL, id))objc_msgSend)(
          menuItem,
          sel_registerName("setRepresentedObject:"),
          ((id (*)(id, SEL, void*))objc_msgSend)((id)objc_getClass("NSValue"), sel_registerName("valueWithPointer:"), m));

      ((id (*)(id, SEL, id))objc_msgSend)(menu, sel_registerName("addItem:"), menuItem);

      if (m->submenu != NULL) {
        ((id (*)(id, SEL, id, id))objc_msgSend)(
            menu,
            sel_registerName("setSubmenu:forItem:"),
            _tray_menu(m->submenu),
            menuItem);
      }
    }
  }

  return menu;
}

static void menu_callback(id self, SEL cmd, id sender) {
  struct tray_menu *m = ((struct tray_menu *(*)(id, SEL))objc_msgSend)(
      ((id (*)(id, SEL))objc_msgSend)(sender, sel_registerName("representedObject")),
      sel_registerName("pointerValue"));

  if (m != NULL && m->cb != NULL) {
    m->cb(m);
  }
}

static int tray_init(struct tray *tray) {
  pool = ((id (*)(id, SEL))objc_msgSend)(
      (id)objc_getClass("NSAutoreleasePool"),
      sel_registerName("new"));

  ((id (*)(id, SEL))objc_msgSend)(
      (id)objc_getClass("NSApplication"),
      sel_registerName("sharedApplication"));

  Class trayDelegateClass = objc_allocateClassPair(objc_getClass("NSObject"), "Tray", 0);
  class_addProtocol(trayDelegateClass, objc_getProtocol("NSApplicationDelegate"));
  class_addMethod(trayDelegateClass, sel_registerName("menuCallback:"), (IMP)menu_callback, "v@:@");
  objc_registerClassPair(trayDelegateClass);

  id trayDelegate = ((id (*)(id, SEL))objc_msgSend)(
      (id)trayDelegateClass,
      sel_registerName("new"));

  app = ((id (*)(id, SEL))objc_msgSend)(
      (id)objc_getClass("NSApplication"),
      sel_registerName("sharedApplication"));

  ((id (*)(id, SEL, id))objc_msgSend)(
      app,
      sel_registerName("setDelegate:"),
      trayDelegate);

  statusBar = ((id (*)(id, SEL))objc_msgSend)(
      (id)objc_getClass("NSStatusBar"),
      sel_registerName("systemStatusBar"));

  statusItem = ((id (*)(id, SEL, double))objc_msgSend)(
      statusBar,
      sel_registerName("statusItemWithLength:"),
      -1.0);

  ((id (*)(id, SEL))objc_msgSend)(statusItem, sel_registerName("retain"));
  ((id (*)(id, SEL, bool))objc_msgSend)(statusItem, sel_registerName("setHighlightMode:"), true);

  statusBarButton = ((id (*)(id, SEL))objc_msgSend)(statusItem, sel_registerName("button"));

  tray_update(tray);

  ((id (*)(id, SEL, bool))objc_msgSend)(
      app,
      sel_registerName("activateIgnoringOtherApps:"),
      true);

  return 0;
}

static int tray_loop(int blocking) {
 id until = blocking
     ? ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSDate"), sel_registerName("distantPast"))
     : ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSDate"), sel_registerName("distantFuture"));

 id event = ((id (*)(id, SEL, unsigned long, id, id, bool))objc_msgSend)(
     app,
     sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:"),
     ULONG_MAX,
     until,
     ((id (*)(id, SEL, const char*))objc_msgSend)(
         (id)objc_getClass("NSString"),
         sel_registerName("stringWithUTF8String:"),
         "kCFRunLoopDefaultMode"),
     true);

 if (event) {
   ((id (*)(id, SEL, id))objc_msgSend)(
       app,
       sel_registerName("sendEvent:"),
       event);
 }

 return 0;
}

static void tray_update(struct tray *tray) {
  ((id (*)(id, SEL, id))objc_msgSend)(
      statusBarButton,
      sel_registerName("setImage:"),
      tray->icon);

  ((id (*)(id, SEL, id))objc_msgSend)(
      statusItem,
      sel_registerName("setMenu:"),
      _tray_menu(tray->menu));
}

static void tray_exit() {}

#elif defined(TRAY_WINAPI)
#include <windows.h>

#include <shellapi.h>

#define WM_TRAY_CALLBACK_MESSAGE (WM_USER + 2)
#define WM_TRAY_PASS_MENU_REF (WM_USER + 1)
#define ID_TRAY_FIRST 1000

static NOTIFYICONDATA nid;
static HWND hwnd;
static HMENU hmenu = NULL;

static std::wstring cstr2wstr(const char* str) {
    int len = MultiByteToWideChar(CP_UTF8, 0, str, strlen(str), nullptr, 0);
    std::wstring ret(len, '\0');
    MultiByteToWideChar(CP_UTF8, 0, str, strlen(str), (LPWSTR)ret.data(), len);
    return ret;
}

static HMENU _tray_menu(struct tray_menu *m, UINT *id) {
  HMENU hmenu = CreatePopupMenu();
  for (; m != NULL && m->text != NULL; m++, (*id)++) {
    if (strcmp(m->text, "-") == 0) {
      InsertMenu(hmenu, *id, MF_SEPARATOR, TRUE, L"");
    } else {
      MENUITEMINFO item;
      memset(&item, 0, sizeof(item));
      item.cbSize = sizeof(MENUITEMINFO);
      item.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE | MIIM_DATA;
      item.fType = 0;
      item.fState = 0;
      if (m->submenu != NULL) {
        item.fMask = item.fMask | MIIM_SUBMENU;
        item.hSubMenu = _tray_menu(m->submenu, id);
      }
      if (m->disabled) {
        item.fState |= MFS_DISABLED;
      }
      if (m->checked) {
        item.fState |= MFS_CHECKED;
      }
      item.wID = *id;
      std::wstring wtext = cstr2wstr(m->text);
      item.dwTypeData = (LPWSTR)wtext.data();
      item.dwItemData = (ULONG_PTR)m;

      InsertMenuItem(hmenu, *id, TRUE, &item);
    }
  }
  return hmenu;
}

static int tray_init(struct tray *tray) {

  hwnd = FindWindow(L"Neutralinojs_webview", NULL);
  if (hwnd == NULL) {
    return -1;
  }
  UpdateWindow(hwnd);

  memset(&nid, 0, sizeof(nid));
  nid.cbSize = sizeof(NOTIFYICONDATA);
  nid.hWnd = hwnd;
  nid.uID = 1000;
  nid.uFlags = NIF_ICON | NIF_MESSAGE;
  nid.uCallbackMessage = WM_TRAY_CALLBACK_MESSAGE;
  Shell_NotifyIcon(NIM_ADD, &nid);
  tray_update(tray);
  return 0;
}

static void tray_recreate() {
  Shell_NotifyIcon(NIM_ADD, &nid);
}

static int tray_loop(int blocking) {
  MSG msg;
  if (blocking) {
    GetMessage(&msg, NULL, 0, 0);
  } else {
    PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
  }
  if (msg.message == WM_QUIT) {
    return -1;
  }
  TranslateMessage(&msg);
  DispatchMessage(&msg);
  return 0;
}

static void tray_update(struct tray *tray) {
  HMENU prevmenu = hmenu;
  UINT id = ID_TRAY_FIRST;
  hmenu = _tray_menu(tray->menu, &id);
  // Send menu ref to webview's Window event listener,
  // because WM_TRAY_CALLBACK_MESSAGE needs it
  SendMessage(hwnd, WM_TRAY_PASS_MENU_REF, (WPARAM)hmenu, 0);
  SendMessage(hwnd, WM_INITMENUPOPUP, (WPARAM)hmenu, 0);
  if (nid.hIcon) {
    DestroyIcon(nid.hIcon);
  }
  nid.hIcon = tray->icon;
  Shell_NotifyIcon(NIM_MODIFY, &nid);

  if (prevmenu != NULL) {
    DestroyMenu(prevmenu);
  }
}

static void tray_exit() {
  Shell_NotifyIcon(NIM_DELETE, &nid);

  if (nid.hIcon) {
    DestroyIcon(nid.hIcon);
  }

  if (hmenu) {
    DestroyMenu(hmenu);
  }
}
#else
#error Please define TRAY_WINAPI, TRAY_APPINDICATOR or TRAY_APPKIT before including this file.
#endif

#endif /* TRAY_H */
#ifdef __cplusplus
}
#endif
