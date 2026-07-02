#ifndef NEU_OS_H
#define NEU_OS_H

#include <string>
#include <functional>

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

struct LocaleInfo {
    string locale = "";
    string language = "";
    string region = "";
};

struct SpawnedProcessEvent {
    int id = -1;
    string type = "";
    string stdIn = "";
};

struct ChildProcessOptions {
    bool background = false;
    bool events = true;
    string cwd = "";
    string stdIn = "";
    map<string, string> envs;
    function<void(const char *bytes, size_t n)> stdOutHandler;
    function<void(const char *bytes, size_t n)> stdErrHandler;
};

bool isTrayInitialized();
void cleanupTray();
void open(const string &url);
os::CommandResult execCommand(string command, const os::ChildProcessOptions &options = {});
pair<int, int> spawnProcess(string command, const os::ChildProcessOptions &options = {});
bool updateSpawnedProcess(const os::SpawnedProcessEvent &evt);
string getPath(const string &name);
os::LocaleInfo getLocaleInfo();
string getEnv(const string &key);
bool setEnv(const string &key, const string &value);
bool trashItem(const string &path);

namespace controllers {

json execCommand(const json &input);
json spawnProcess(const json &input);
json updateSpawnedProcess(const json &input);
json getSpawnedProcesses(const json &input);
json getEnv(const json &input);
json setEnv(const json &input);
json getEnvs(const json &input);
json getLocaleInfo(const json &input);
json showOpenDialog(const json &input);
json showFolderDialog(const json &input);
json showSaveDialog(const json &input);
json showNotification(const json &input);
json showMessageBox(const json &input);
json setTray(const json &input);
json open(const json &input);
json getPath(const json &input);
json trashItem(const json &input);

} // namespace controllers

} // namespace os

#endif // #define NEU_OS_H
