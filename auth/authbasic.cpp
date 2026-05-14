#include <string>
#include <filesystem>
#include <cstring>

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

    // Restrict file permissions to owner-only (0600) to prevent
    // other local users from reading the auth token.
    try {
        filesystem::permissions(CONVSTR(tempAuthInfoPath),
            filesystem::perms::owner_read | filesystem::perms::owner_write,
            filesystem::perm_options::replace);
    }
    catch(const filesystem::filesystem_error& e) {
        debug::log(debug::LogTypeError, "Failed to set permissions on " + tempAuthInfoPath);
    }

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

// Constant-time comparison to prevent timing side-channel attacks.
// Unlike std::string::operator== which short-circuits on the first
// mismatch, this always compares the full length.
bool __constantTimeCompare(const string &a, const string &b) {
    if(a.size() != b.size())
        return false;
    volatile unsigned char result = 0;
    for(size_t i = 0; i < a.size(); i++) {
        result |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
    }
    return result == 0;
}

bool verifyToken(const string &accessToken) {
    return __constantTimeCompare(token, accessToken);
}

bool verifyConnectToken(const string &inConnectToken) {
    return __constantTimeCompare(connectToken, inConnectToken);
}

} // namespace authbasic
