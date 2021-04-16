#ifndef AP_H
#define AP_H

#include <iostream>
#include <map>

using namespace std;

namespace app {
    string exit(json input);
    string keepAlive(json input);
    string showWindow(json input);
    string open(json input);
    string getConfig(json input);
}

#endif
