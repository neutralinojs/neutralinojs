#ifndef WI_H
#define WI_H
#include "lib/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace window {
    void executeJavaScript(string js);
    bool isMaximized();
    void maximize();
    void minimize();

namespace controllers {
    json show(json input);
    json setTitle(json input);
    json maximize(json input); 
    json unmaximize(json input);
    json isMaximized(json input);
    json minimize(json input);
        
} // namespace controllers
} // namespace window

#endif
