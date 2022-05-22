#ifndef NEU_OS_H
#define NEU_OS_H

#include <string>

#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace os {

struct CommandResult {
    int pid = -1;
    int exitCode = -1;
    string stdErr = "";
    string stdOut = "";
};

struct SpawnedProcessEvent {
    int id = -1;
    string type = "";
    string stdIn = "";
};

bool isTrayInitialized();
void cleanupTray();
void open(const string &url);
os::CommandResult execCommand(string command, const string &input = "", bool background = false);
int spawnProcess(string command);
bool updateSpawnedProcess(const os::SpawnedProcessEvent &evt);
string getPath(const string &name);
string getEnv(const string &key);

namespace controllers {

json execCommand(const json &input);
json spawnProcess(const json &input);
json updateSpawnedProcess(const json &input);
json getSpawnedProcesses(const json &input);
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
