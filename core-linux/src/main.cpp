// MIT License

// Copyright (c) 2018 Neutralinojs

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#define WEBVIEW_IMPLEMENTATION
#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>
#include <map>
#include "../lib/json/json.hpp"
#include "functions.h"
#include "settings.h"
#include "resources.h"
#include "Socket.h"
#include "Handler.h"
#include "auth/authbasic.h"
#include "ping/ping.h"
#include "cloud/privileges.h"
#include "webview.h"
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

using namespace std;
using json = nlohmann::json;

std::map<int, std::thread> threads;

void uiThread(string appname, int port, int width, int height,
              int fullscreen, string title, bool always_on_top, GdkPixbuf* icon,
              int enable_inspector, bool borderless_window, bool maximize, string url)
{
  struct webview webview;
  memset(&webview, 0, sizeof(webview));
  webview.title = title.c_str();
  webview.url = url.c_str();
  webview.width = width;
  webview.height = height;
  webview.resizable = 1;
  webview.always_on_top = always_on_top;
  webview.icon = icon;
  webview.debug = enable_inspector;
  webview.borderless_window = borderless_window;
  webview.maximize = maximize;
  int r = webview_init(&webview);
  webview_set_fullscreen(&webview, fullscreen);
  if (r != 0)
  {
    return;
  }
  while (webview_loop(&webview, 1) == 0)
  {
  }
  webview_exit(&webview);
}

int main(int argc, char **argv)
{
  json args;
  for(int i = 0; i < argc; i++) {
    args.push_back(argv[i]);
  }
  settings::setGlobalArgs(args);
  resources::makeFileTree();
  json options = settings::getSettings();
  authbasic::generateToken();
  ping::startPingReceiver();
  privileges::getMode();
  privileges::getBlacklist();

  int port = stoi(options["appport"].get<string>());
  string appname = options["appname"].get<std::string>();
  string mode = privileges::getMode();

  int listenFd = Socket::createSocket();
  Socket::setReuseAddr(listenFd, true);
  struct sockaddr_in servAddr;
  memset(&servAddr, 0, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(port);
  Socket::Bind(listenFd, servAddr);

  struct sockaddr_in sin;
  socklen_t len = sizeof(sin);
  if (getsockname(listenFd, (struct sockaddr *)&sin, &len) == -1)
  {
    perror("getsockname");
  }
  else
  {
    port = ntohs(sin.sin_port);
    settings::setOption("appport", std::to_string(port));
  }
  string navigateUrl = ("http://localhost:" + std::to_string(port) + "/" + appname);
  if(!options["url"].is_null() && options["url"].get<string>() != "/")
    navigateUrl = options["url"];
  Socket::Listen(listenFd);

  if (mode == "browser")
  {
    system(("xdg-open \"" + navigateUrl + "\"").c_str());
  }
  else if (mode == "window")
  {
    int width = 800;
    int height = 600;
    int fullscreen = 0;
    bool is_always_on_top = false;
    GdkPixbuf* icon = nullptr;
    std::string title = "Neutralino window";
    int enable_inspector = 0;
    bool is_borderless_window = false;
    bool maximize = false;
    if (!options["window"].is_null())
    {
      json windowProp = options["window"];
      width = stoi(windowProp["width"].get<std::string>());
      height = stoi(windowProp["height"].get<std::string>());
      if (!windowProp["fullscreen"].is_null())
        fullscreen = windowProp["fullscreen"].get<bool>() ? 1 : 0;

      if (!windowProp["alwaysontop"].is_null())
        is_always_on_top = windowProp["alwaysontop"].get<bool>();

      if (!windowProp["title"].is_null())
        title = windowProp["title"].get<std::string>();

      if (!windowProp["iconfile"].is_null()) {
        GdkPixbufLoader *loader;
        GdkPixbuf *pixbuf;
        loader = gdk_pixbuf_loader_new ();
        std::string iconFile = windowProp["iconfile"].get<std::string>();
        std::string iconDataStr = settings::getFileContent(iconFile);

        const char *iconData = iconDataStr.c_str();
        unsigned char *uiconData = reinterpret_cast<unsigned char*>(const_cast<char*>(iconData));
        gdk_pixbuf_loader_write(loader, uiconData, iconDataStr.length(), NULL);
        icon = gdk_pixbuf_loader_get_pixbuf(loader);
      }

      if (!windowProp["enableinspector"].is_null())
        enable_inspector = windowProp["enableinspector"].get<bool>() ? 1 : 0;

      if (!windowProp["borderless"].is_null())
        is_borderless_window = windowProp["borderless"].get<bool>();

      if (!windowProp["maximize"].is_null())
        maximize = windowProp["maximize"].get<bool>();
    }
    std::thread ren(uiThread, appname, port, width,
                    height, fullscreen, title, is_always_on_top, icon,
                    enable_inspector, is_borderless_window, maximize, navigateUrl);
    ren.detach();
  }

  while (true)
  {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    memset(&clientAddr, 0, sizeof(clientAddr));
    int connFd = Socket::Accept(listenFd, &clientAddr);
    threads[connFd] = std::thread(Handler::handle, connFd);
    threads[connFd].detach();
  }
  Socket::Close(listenFd);

  return 0;
}
