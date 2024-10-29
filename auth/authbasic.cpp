#include <string>
#include <filesystem>
#include <vector>

#include "helpers.h"
#include "settings.h"
#include "auth/authbasic.h"
#include "api/fs/fs.h"
#include "api/debug/debug.h"

using namespace std;

namespace authbasic {

string token = "";
string connectToken = "";
authbasic::TokenSecurity tokenSecurity = authbasic::TokenSecurityOneTime;
bool tokenSent = false;

json __makeAuthInfoPayload() {
    json info;
    info["nlToken"] = authbasic::getTokenInternal();
    info["nlConnectToken"] = authbasic::getConnectTokenInternal();
    info["nlPort"] = settings::getOptionForCurrentMode("port").get<int>();
    return info;
}

void init() {
    token = helpers::generateToken();
    connectToken = helpers::splitTwo(token, '.')[1];
    json jTokenSecurity = settings::getOptionForCurrentMode("tokenSecurity");
    if(!jTokenSecurity.is_null()) {
        tokenSecurity = jTokenSecurity.get<string>() == "one-time"
                            ? authbasic::TokenSecurityOneTime : authbasic::TokenSecurityNone;
    }
}

void exportAuthInfo() {
    string tempDirPath = settings::joinAppPath("/.tmp");
    filesystem::create_directories(CONVSTR(tempDirPath));
    string tempAuthInfoPath = settings::joinAppPath("/.tmp/auth_info.json");
    fs::FileWriterOptions fileWriterOptions = {
        tempAuthInfoPath,
        __makeAuthInfoPayload().dump()
    };
    fs::writeFile(fileWriterOptions);

    debug::log(debug::LogTypeInfo, "Auth info was exported to " + tempAuthInfoPath);
}

string getToken() {
    if(tokenSent && tokenSecurity == authbasic::TokenSecurityOneTime) {
        return "";
    }
    tokenSent = true;
    return authbasic::getTokenInternal();
}

string getTokenInternal() {
    return token;
}

string getConnectTokenInternal() {
    return connectToken;
}

bool verifyToken(const string &accessToken) {
    return token == accessToken;
}

bool verifyConnectToken(const string &inConnectToken) {
    return connectToken == inConnectToken;
}

// my comparison to the original code is that I have added the json &input parameter to the function
bool verifyToken(const jason &input) {
    vector<string> requiredFields = {"token"};
    if(!helpers::hasRequiredFields(input, requiredFields)) {
        return false;
    }
    return authbasic::verifyToken(input["token"].get<string>());
}

bool verifyConnectToken(const json &output) {
    vector<string> requiredFields = {"connectToken"};
    if(!helpers::hasRequiredFields(output, requiredFields)) {
        return false;
    }
    return authbasic::verifyConnectToken(output["connectToken"].get<string>());
}

} // namespace authbasic
