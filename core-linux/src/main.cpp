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
#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>
#include <regex>
#include <map>
#include "lib/json.hpp"
#include "helpers.h"
#include "settings.h"
#include "resources.h"
#include "server/Socket.h"
#include "server/Handler.h"
#include "auth/authbasic.h"
#include "ping/ping.h"
#include "permission.h"
#include "api/app/app.h"

using namespace std;
using json = nlohmann::json;

map < int, thread > threads;

int main(int argc, char ** argv) {
    json args;
    for (int i = 0; i < argc; i++) {
        args.push_back(argv[i]);
    }
    settings::setGlobalArgs(args);
    if (!loadResFromDir)
        resources::makeFileTree();
    json options = settings::getConfig();
    authbasic::generateToken();
    ping::startPingReceiver();
    permission::registerBlockList();

    int port = 0;
    if(!options["port"].is_null())
        port = options["port"];
    string mode = settings::getMode();

    int listenFd = Socket::createSocket();
    Socket::setReuseAddr(listenFd, true);
    struct sockaddr_in servAddr;
    memset( & servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    if(mode == "cloud")
        servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servAddr.sin_port = htons(port);
    Socket::Bind(listenFd, servAddr);

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(listenFd, (struct sockaddr * ) & sin, & len) == -1) {
        perror("getsockname");
    }
    else {
        port = ntohs(sin.sin_port);
        settings::setPort(port);
    }
    string navigationUrl = "http://localhost:" + std::to_string(port);
    if(!options["url"].is_null()) {
        string url = options["url"];
        if (regex_match(url, regex("^/.*")))
            navigationUrl += url;
        else
            navigationUrl = url;
    }
    Socket::Listen(listenFd);

    if(mode == "browser") {
        json browserOptions = options["modes"]["browser"];
        browserOptions["url"] = navigationUrl;
        app::open(browserOptions);
    }
    else if(mode == "window") {
        json windowOptions = options["modes"]["window"];
        windowOptions["url"] = navigationUrl;
        app::showWindow(windowOptions);
    }

    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        memset( & clientAddr, 0, sizeof(clientAddr));
        int connFd = Socket::Accept(listenFd, & clientAddr);
        threads[connFd] = std::thread(Handler::handle, connFd);
        threads[connFd].detach();
    }
    Socket::Close(listenFd);
    return 0;
}
