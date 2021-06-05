#ifndef AP_H
#define AP_H

using namespace std;

namespace app {
    string exit(json input);
    string open(json input);
    string keepAlive(json input);
    string getConfig(json input);
}

#endif
