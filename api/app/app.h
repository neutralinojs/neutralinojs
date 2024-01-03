#ifndef NEU_APP_H
#define NEU_APP_H

#include <string>

#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace app {

void exit(int code = 0);
unsigned int getProcessId();

namespace controllers {

json exit(const json &input);
json killProcess(const json &input);
json getConfig(const json &input);
json broadcast(const json &input);
json readProcessInput(const json &input);
json writeProcessOutput(const json &input);
json writeProcessError(const json &input);

} // namespace controllers

} // namespace app

#endif // define NEU_APP_H
