#include "api/window/window.h"

using namespace std;

namespace events {
    void dispatch(string event, string data) {
        string js = "Neutralino.events.dispatch('" + event + 
                    "', " + data + ");";
    	window::executeJavaScript(js);
    }
} // namespace events
