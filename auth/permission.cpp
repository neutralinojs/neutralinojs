#include <string>
#include <vector>
#include <regex>
#include <algorithm>

#include "lib/json/json.hpp"
#include "settings.h"
#include "helpers.h"

using namespace std;
using json = nlohmann::json;

namespace permission {

vector<string> blockedMethods;
vector<string> blockedModules;
vector<string> allowedMethods;
vector<string> allowedModules;
bool shouldCheckBlockList = false;
bool shouldCheckAllowList = false;

bool __isWildcardMatch(const string &methodMatch) {
    return regex_match(methodMatch, regex(".*\\.\\*"));
}

string __getModuleFromMethod(const string &nativeMethod) {
    vector<string> methodParts = helpers::splitTwo(nativeMethod, '.');
    return methodParts[0];
}

void __registerBlockList() {
    json jNativeBlockList = settings::getOptionForCurrentMode("nativeBlockList");
    if(jNativeBlockList.is_null())
        return;
    vector<string> blockListVector = jNativeBlockList.get<vector<string>>();

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
    json jNativeAllowList = settings::getOptionForCurrentMode("nativeAllowList");
    if(jNativeAllowList.is_null())
        return;
    vector<string> allowListVector = jNativeAllowList.get<vector<string>>();

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

bool hasMethodAccess(const string &nativeMethod) {
    string module = __getModuleFromMethod(nativeMethod);

    // If a block list is configured, reject blocked methods first.
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
    }

    // If an allow list is configured, the method must be explicitly allowed.
    // This check runs regardless of whether a block list is also configured,
    // so both lists are honored when both are set.
    if(shouldCheckAllowList) {
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

    // If only a block list is configured and we passed it, allow.
    // If neither list is configured, allow everything.
    return true;
}

bool hasAPIAccess() {
    json jEnableNativeAPI = settings::getOptionForCurrentMode("enableNativeAPI");
    if(!jEnableNativeAPI.is_null())
        return jEnableNativeAPI.get<bool>();
    return false;
}

void init() {
    __registerAllowList();
    __registerBlockList();
}

} // namespace permission
