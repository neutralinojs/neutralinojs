#ifndef NEU_APP_H
#define NEU_APP_H

#include <string>
#include <vector>

#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace app {

struct AutoStartOptions {
    bool enabled = false;
    string path = "";
    vector<string> args = {};
    string name = "";
};

void exit(int code = 0);
unsigned int getProcessId();
bool setAutoStart(const AutoStartOptions &options);

namespace controllers {

json exit(const json &input);
json killProcess(const json &input);
json getProcessId(const json &input);
json setAutoStart(const json &input);
json getConfig(const json &input);
json broadcast(const json &input);
json readProcessInput(const json &input);
json writeProcessOutput(const json &input);
json writeProcessError(const json &input);

} // namespace controllers

} // namespace app

#endif // define NEU_APP_H
