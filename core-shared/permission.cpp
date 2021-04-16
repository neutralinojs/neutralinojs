#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include "lib/json.hpp"
#include "settings.h"

using namespace std;
using json = nlohmann::json;

namespace permission {
    vector <string> blockList;

    void registerBlockList() {
        if(settings::getConfig()["nativeBlockList"].is_null())
            return;
        json blockListOptions = settings::getConfig()["nativeBlockList"];
        vector<string> blockListVector = blockListOptions;
        blockList = blockListVector;
    }

    bool hasAccess(string nativeMethod) {
        for(int i = 0; i < blockList.size(); i++) {
            if(blockList[i] == nativeMethod)
                return false;
        }
        return true;
    }
}
