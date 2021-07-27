#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <regex>
#include <algorithm>
#include "lib/json.hpp"
#include "settings.h"
#include "helpers.h"

using namespace std;
using json = nlohmann::json;

namespace permission {
    vector <string> blockedMethods;
    vector <string> blockedModules;
    
    bool __isWildcardMatch(string methodMatch) {
        return regex_match(methodMatch, regex(".*\\.\\*"));
    }
    
    bool __isPingMethod(string methodMatch) {
        return methodMatch == "app.keepAlive";
    }
    
    string __getModuleFromMethod(string nativeMethod) {
        vector <string> methodParts = helpers::split(nativeMethod, '.');
        return methodParts[0];
    }

    void registerBlockList() {
        json config = settings::getConfig();
        if(config["nativeBlockList"].is_null())
            return;
        json blockListOptions = config["nativeBlockList"];
        vector<string> blockListVector = blockListOptions;
        
        for(int i = 0; i < blockListVector.size(); i++) {
            // Adding blocked modules
            if(__isWildcardMatch(blockListVector[i])) {
                blockedModules.push_back(__getModuleFromMethod(blockListVector[i]));
            }
            // Adding blocked methods
            else {
                blockedMethods.push_back(blockListVector[i]);
            }
        }
    }

    bool hasMethodAccess(string nativeMethod) {
        if(__isPingMethod(nativeMethod))
            return true;
        string module = __getModuleFromMethod(nativeMethod);
        // Check modules
        if(find(blockedModules.begin(), blockedModules.end(), module) 
                != blockedModules.end()) {
            return false;        
        }
        
        // Check methods
        if(find(blockedMethods.begin(), blockedMethods.end(), nativeMethod) 
                != blockedMethods.end()) {
            return false;        
        }
        return true;
    }

    bool hasAPIAccess(string nativeMethod) {
        if(__isPingMethod(nativeMethod))
            return true;
        if(!settings::getConfig()["enableNativeAPI"].is_null())
            return settings::getConfig()["enableNativeAPI"].get<bool>();
        return false;
    }
}
