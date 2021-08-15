#ifndef AP_H
#define AP_H

using namespace std;

namespace app {
    void exit();
    void open(string url);

namespace controllers {
    json exit(json input);
    json killProcess(json input);
    json open(json input);
    json keepAlive(json input);
    json getConfig(json input);

} // namespace controllers
} // namespace app
#endif
