#ifndef OS_H
#define OS_H

#include <string>

#include "lib/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace os {
    void open(string url);
    string execCommand(string command, bool shouldCombineErrorStream = false,
                        bool shouldRunInBackground = false);
    string getPath(string name);

namespace controllers {
    json execCommand(json input);
    json getEnv(json input);
    json showOpenDialog(json input);
    json showFolderDialog(json input);
    json showSaveDialog(json input);
    json showNotification(json input);
    json showMessageBox(json input);
    json setTray(json input);
    json open(json input);
    json getPath(json input);
} // namespace controllers
} // namespace os

#endif
