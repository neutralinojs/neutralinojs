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
#include "Socket.h"
#include "Handler.h"
#include "serverlistener.h"
#include "auth/authbasic.h"
#include "ping/ping.h"
#include "permission.h"

std::string ServerListener::init() {
    int port = 0;
    json options = settings::getConfig();
    if(!options["port"].is_null())
        port = options["port"];
    string mode = settings::getMode();

    this->listenFd = Socket::createSocket();
    Socket::setReuseAddr(this->listenFd, true);
    struct sockaddr_in servAddr;
    memset( & servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    if(mode == "cloud")
        servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servAddr.sin_port = htons(port);
    Socket::Bind(this->listenFd, servAddr);

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(this->listenFd, (struct sockaddr * ) & sin, & len) == -1) {
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
    Socket::Listen(this->listenFd);
    return navigationUrl;
}

void ServerListener::run() {
    map <int, thread> threads;
    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        memset( & clientAddr, 0, sizeof(clientAddr));
        int connFd = Socket::Accept(this->listenFd, & clientAddr);
        threads[connFd] = std::thread(Handler::handle, connFd);
        threads[connFd].detach();
    }
}

void ServerListener::stop() {
    Socket::Close(this->listenFd);
}

