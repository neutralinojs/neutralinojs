#ifndef WI_H
#define WI_H
#include "lib/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace window {
    string show(json input);
    string setTitle(json input);

    void _executeJavaScript(string js);

    namespace WindowMethods{
        string __navigate(json input);

        string maximize(json input); 
        
        string unmaximize(json input);

        string iconify(json input);

        string deiconify(json input);

        string destroy(json input);
    }
}

#endif
