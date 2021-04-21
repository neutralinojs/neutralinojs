#ifndef SERVERLISTENER_H
#define SERVERLISTENER_H

#include <string>

class ServerListener {
    int listenFd;
public:
    string init();
    void run();
    void stop();
};

#endif // SERVERLISTENER_H
