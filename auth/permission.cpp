#include <string>
#include <vector>
#include <regex>
#include <algorithm>
#include <filesystem>

#include "lib/json/json.hpp"
#include "settings.h"
#include "auth/permission.h"
#include "helpers.h"
#include "api/fs/fs.h"

using namespace std;
using json = nlohmann::json;

namespace permission {

vector<string> blockedMethods;
vector<string> blockedModules;
vector<string> allowedMethods;
vector<string> allowedModules;
vector<string> allowedCommandPatterns;
vector<string> fileSystemReadScopes;
vector<string> fileSystemWriteScopes;
bool shouldCheckBlockList = false;
bool shouldCheckAllowList = false;
bool shouldCheckCommandAllowList = false;
bool shouldCheckScopesLists = false;

bool __isWildcardMatch(const string &methodMatch) {
    return regex_match(methodMatch, regex(".*\\.\\*"));
}

string __getModuleFromMethod(const string &nativeMethod) {
    vector<string> methodParts = helpers::splitTwo(nativeMethod, '.');
    return methodParts[0];
}

string __normalizeScopePath(const string &path) {
    string resolved = fs::applyPathConstants(path);
    string norm = resolved;
    norm = FS_CONVWSTRN(filesystem::weakly_canonical(norm));
    norm = FS_CONVWSTRN(filesystem::absolute(norm));
    return helpers::normalizePath(norm);
}

bool __isPathInScope(const string &originalPath, const string &scope) {
    string path = __normalizeScopePath(originalPath);
    string targetScope = scope;
    #if defined(_WIN32)
    transform(path.begin(), path.end(), path.begin(), ::tolower);
    transform(targetScope.begin(), targetScope.end(), targetScope.begin(), ::tolower);
    #endif
    if(path == targetScope) {
        return true;
    }
    if(path.size() > targetScope.size()
            && path.compare(0, targetScope.size(), targetScope) == 0
            && path[targetScope.size()] == '/') {
        return true;
    }
    return false;
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

void __regiserCommandAllowList() {
    json jCommandAllowList = settings::getOptionForCurrentMode("commandAllowList");
    if(jCommandAllowList.is_null())
        return;
    allowedCommandPatterns = jCommandAllowList.get<vector<string>>();
    shouldCheckCommandAllowList = true;
}

void __registerScopesLists() {
    json jScopes = settings::getOptionForCurrentMode("filesystemScopes");
    if(jScopes.is_null())
        return;

    for(const auto &entry: jScopes.items()) {
        string perm = entry.value().get<string>();
        string normalizedScope = __normalizeScopePath(entry.key());
        if(perm == "read-write" || perm == "read") {
            fileSystemReadScopes.push_back(normalizedScope);
        }
        if(perm == "read-write" || perm == "write") {
            fileSystemWriteScopes.push_back(normalizedScope);
        }
    }

    shouldCheckScopesLists = true;
}

bool hasMethodAccess(const string &nativeMethod) {
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

bool hasAPIAccess() {
    json jEnableNativeAPI = settings::getOptionForCurrentMode("enableNativeAPI");
    if(!jEnableNativeAPI.is_null())
        return jEnableNativeAPI.get<bool>();
    return false;
}

bool hasCommandExecutionAccess(const string &command) {
    if(!shouldCheckCommandAllowList || allowedCommandPatterns.empty()) {
        return true;
    }

    vector<string> tokens = helpers::tokenizeCommand(command);
    if(tokens.empty()) {
        return false;
    }

    string program = tokens[0];
    for(const string &pattern: allowedCommandPatterns) {
        if(helpers::globalMatch(program, pattern)) {
            return true;
        }
    }
    return false;
}


bool hasFileSystemPathAccess(const string &path, permission::FileSystemAccessPermission perm) {
    if(!shouldCheckScopesLists) {
        return true;
    }

    const auto activeScopes = (perm == permission::FileSystemAccessPermissionWrite)
        ? fileSystemWriteScopes : fileSystemReadScopes;

    if(activeScopes.empty()) {
        return false;
    }

    for(const string &scope: activeScopes) {
        if(__isPathInScope(path, scope)) {
            return true;
        }
    }
    return false;
}

void init() {
    __registerAllowList();
    __registerBlockList();
    __regiserCommandAllowList();
    __registerScopesLists();
}

} // namespace permission
