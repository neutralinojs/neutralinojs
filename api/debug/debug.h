#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <map>

using namespace std;

namespace debug {
    void log(string type, string message);

namespace controllers {
    json log(json input);
} // namespace controllers
} // namespace debug

#endif
