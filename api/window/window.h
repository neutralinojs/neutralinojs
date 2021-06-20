#ifndef WI_H
#define WI_H
#include "lib/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace window {
    string show(json input);
    string setTitle(json input);
    void _executeJavaScript(string js);
}

#endif
