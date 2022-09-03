#include <string>

#include "helpers.h"
#include "settings.h"
#include "auth/authbasic.h"
#include "api/filesystem/filesystem.h"
#include "api/debug/debug.h"

using namespace std;

namespace authbasic {

string token = "";
authbasic::TokenSecurity tokenSecurity = authbasic::TokenSecurityOneTime;
bool tokenSent = false;

json __makeAuthInfoPayload() {
    json info;
    info["accessToken"] = token;
    info["port"] = settings::getOptionForCurrentMode("port").get<int>();
    return info;
}

void init() {
    token = helpers::generateToken();
    json jTokenSecurity = settings::getOptionForCurrentMode("tokenSecurity");
    if(!jTokenSecurity.is_null()) {
        tokenSecurity = jTokenSecurity.get<string>() == "one-time"
                            ? authbasic::TokenSecurityOneTime : authbasic::TokenSecurityNone;
    }
}

void exportAuthInfo() {
    string tempDirPath = settings::joinAppPath("/.tmp");
    fs::createDirectory(tempDirPath);
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

bool verifyToken(const string &accessToken) {
    return token == accessToken;
}

} // namespace authbasic
