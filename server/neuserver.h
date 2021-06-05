#ifndef NEUSERVER_H
#define NEUSERVER_H

#include <string>
#include "lib/httplib.h"

using namespace std;

class NeuServer {
    httplib::Server *svr;
    int port = 0;
    bool usingRandomPort = false;
    string hostName = "127.0.0.1";

public:
    string init();
    void run();
    void stop();
};

#endif // NEUSERVER_H
