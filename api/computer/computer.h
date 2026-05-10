#ifndef NEU_COMPUTER_H
#define NEU_COMPUTER_H

#include <string>
#include <vector>
#include <utility>
#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace computer {

enum SendKeyState { SendKeyStatePress, SendKeyStateDown, SendKeyStateUp };

bool setMousePosition(int x, int y);
bool setMouseGrabbing(bool grabbing);
bool sendKey(const string &key, computer::SendKeyState keyState);

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
json setMousePosition(const json &input);
json setMouseGrabbing(const json &input);
json sendKey(const json &input);;

} // namespace controllers
} // namespace computer

#endif
