#ifndef OS_H
#define OS_H

#include "lib/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace os {
    struct MessageBoxOptions {
        string type;
        string title;
        string content;
    };
    
    struct MessageBoxResult {
        bool hasError = false;
        string error;
        bool yesButtonClicked = false;
    };

    void open(string url);
    string execCommand(string command, bool shouldCombineErrorStream = false,
                        bool shouldRunInBackground = false);
    MessageBoxResult showMessageBox(MessageBoxOptions options);

namespace controllers {
    json execCommand(json input);
    json getEnv(json input);
    json showOpenDialog(json input);
    json showSaveDialog(json input);
    json showNotification(json input);
    json showMessageBox(json input);
    json setTray(json input);
    json open(json input);
} // namespace controllers
} // namespace os

#endif
