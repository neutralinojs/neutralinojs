#ifndef NEU_COMPUTER_H
#define NEU_COMPUTER_H

#include <string>
#include <vector>
#include <utility>
#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace computer {

enum class CoordSpace {
    Window,
    Screen
};

struct InputCapabilities {
    bool warp = false;
    bool grab = false;
    bool syntheticKeys = false;
};

bool setCursorPosition(int x, int y, CoordSpace space);
bool setCursorGrab(bool enabled);
bool sendKey(const string &key,
             const vector<string> &mods,
             const string &state);
InputCapabilities getInputCapabilities();

string getArch();
pair<int, int> getMousePosition();

namespace controllers {

json getMemoryInfo(const json &input);
json getArch(const json &input);
json getKernelInfo(const json &input);
json getOSInfo(const json &input);
json getCPUInfo(const json &input);
json getDisplays(const json &input);
json getMousePosition(const json &input);

json setCursorPosition(const json &input);
json setCursorGrab(const json &input);
json sendKey(const json &input);
json getInputCapabilities(const json &input);

} // namespace controllers
} // namespace computer

#endif
