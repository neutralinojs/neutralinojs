#include <iostream>
#include <cstdlib>
#include <string>
#include "functions.h"
#include "settings.h"
#include "Socket.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"
#include "auth/authbasic.h"
#include "ping/ping.h"


using namespace std;

int main(int argc, char **argv)
{
    json options = settings::getSettings();
    authbasic::generateToken();
    ping::startPingReceiver();

    int port = stoi(options["appport"].get<string>());
    string appname = options["appname"].get<std::string>();

    system(("xdg-open http://localhost:" + std::to_string(port) + "/" + appname).c_str() );

    
    
    int listenFd = Socket::createSocket();
    Socket::setReuseAddr(listenFd, true);
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);
    Socket::Bind(listenFd, servAddr);
    Socket::Listen(listenFd);

    EventLoopThreadPool *threadPool = new EventLoopThreadPool(4);
    while(true)
    {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        memset(&clientAddr, 0, sizeof(clientAddr));
        int connFd = Socket::Accept(listenFd, &clientAddr);

        EventLoopThread *thread = threadPool->getNextThread();
        EventLoop *loop = thread->getLoop();
        loop->addToLoop(connFd);
    }
    Socket::Close(listenFd);
    delete threadPool;
    return 0;
}