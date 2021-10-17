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
    json exit(json input);
    json killProcess(json input);
    json keepAlive(json input);
    json getConfig(json input);

} // namespace controllers
} // namespace app
#endif // define NEU_APP_H
