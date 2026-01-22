#ifndef NEU_API_INPUT_H
#define NEU_API_INPUT_H

#include "lib/json/json.hpp"

using json = nlohmann::json;

namespace input {

    enum class CoordSpace {
        Window,
        Screen
    };

    struct InputCapabilities {
        bool warp = false;
        bool grab = false;
        bool syntheticKeys = false;
    };

    // Native layer (platform-specific later)
    bool setCursorPositionNative(int x, int y, CoordSpace space);
    bool setCursorGrabNative(bool enabled);
    bool sendKeyNative(const std::string &key,
                       const std::vector<std::string> &mods,
                       const std::string &state);
    InputCapabilities getCapabilitiesNative();

    namespace controllers {
        json setCursorPosition(const json &input);
        json setCursorGrab(const json &input);
        json sendKey(const json &input);
        json getCapabilities(const json &input);
    }

}

#endif
