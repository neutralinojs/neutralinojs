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
    vector <string> allowedMethods;
    vector <string> allowedModules;
    bool shouldCheckBlockList = false;
    bool shouldCheckAllowList = false;
    
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

    void __registerBlockList() {
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
        shouldCheckBlockList = true;
    }
    
    void __registerAllowList() {
        json config = settings::getConfig();
        if(config["nativeAllowList"].is_null())
            return;
        json allowListOptions = config["nativeAllowList"];
        vector<string> allowListVector = allowListOptions;
        
        for(int i = 0; i < allowListVector.size(); i++) {
            // Adding allowed modules
            if(__isWildcardMatch(allowListVector[i])) {
                allowedModules.push_back(__getModuleFromMethod(allowListVector[i]));
            }
            // Adding allowed methods
            else {
                allowedMethods.push_back(allowListVector[i]);
            }
        }
        shouldCheckAllowList = true;
    }

    bool hasMethodAccess(string nativeMethod) {
        if(__isPingMethod(nativeMethod))
            return true;
        string module = __getModuleFromMethod(nativeMethod);
        if(shouldCheckBlockList) {
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
            return true; // method is not blocked
        }
        else if(shouldCheckAllowList) {
            // Check modules
            if(find(allowedModules.begin(), allowedModules.end(), module) 
                    != allowedModules.end()) {
                return true;        
            }
            
            // Check methods
            if(find(allowedMethods.begin(), allowedMethods.end(), nativeMethod) 
                    != allowedMethods.end()) {
                return true;        
            }
            return false; // method is not allowed
        }
        return true; // anything is allowed if no allow/block list defined
    }

    bool hasAPIAccess(string nativeMethod) {
        if(__isPingMethod(nativeMethod))
            return true;
        if(!settings::getConfig()["enableNativeAPI"].is_null())
            return settings::getConfig()["enableNativeAPI"].get<bool>();
        return false;
    }
    
    void init() {
        __registerAllowList();
        __registerBlockList();
    }
}
