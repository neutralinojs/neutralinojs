#ifndef NEU_OS_H
#define NEU_OS_H

#include <string>

#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace os {
    void open(const string &url);
    string execCommand(string command, bool shouldCombineErrorStream = false,
                        bool shouldRunInBackground = false);
    string getPath(const string &name);

namespace controllers {
    json execCommand(const json &input);
    json getEnv(const json &input);
    json showOpenDialog(const json &input);
    json showFolderDialog(const json &input);
    json showSaveDialog(const json &input);
    json showNotification(const json &input);
    json showMessageBox(const json &input);
    json setTray(const json &input);
    json open(const json &input);
    json getPath(const json &input);
} // namespace controllers
} // namespace os

#endif // #define NEU_OS_H
