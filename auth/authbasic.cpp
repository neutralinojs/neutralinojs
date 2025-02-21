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
    string tempDirPath = settings::joinAppDataPath("/.tmp");
    try {
        filesystem::create_directories(CONVSTR(tempDirPath));
    } 
    catch (const filesystem::filesystem_error& e) {
        debug::log(debug::LogTypeError, "Failed to create " + tempDirPath);
        return;
    }
    string tempAuthInfoPath = settings::joinAppDataPath("/.tmp/auth_info.json");
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

} // namespace authbasic
