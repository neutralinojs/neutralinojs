#include <string>
#include <filesystem>

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

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

    // Security fix for GHSA-jrpq-x4rc-3w47 (CWE-732):
    // Insecure file permissions on auth token export file and directory.
    //
    // Threat Model:
    // When `exportAuthInfo` is enabled, Neutralino exports the native server port (`nlPort`),
    // the native access token (`nlToken`), and the WebSocket connect token (`nlConnectToken`)
    // into `.tmp/auth_info.json`. On multi-user systems or shared environments, if this file
    // or its parent directory are created with default umask permissions (mode 0755 / 0644),
    // any unprivileged local user or process on the machine can read these tokens and gain
    // full control of the application's native backend via the WebSocket interface.
    //
    // Defensive Requirements:
    // 1. Directory permissions: The `.tmp` directory must be restricted to owner-only access
    //    (mode 0700: rwx------) so other users cannot enter or list files in the directory.
    // 2. File permissions: The `auth_info.json` file must be created with owner-only access
    //    (mode 0600: rw-------) so other users cannot read the connection tokens.
    // 3. Atomicity (No TOCTOU race condition): We must NOT create the file or folder with world-readable
    //    permissions and subsequently tighten them via chmod, as an attacker could open the file handle
    //    in the intermediate gap. On POSIX, we create the directory with a restrictive umask (0077)
    //    and the file via open() with O_CREAT and mode 0600 directly.
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
    mode_t oldMask = umask(0077);
    try {
        filesystem::create_directories(CONVSTR(tempDirPath));
    } 
    catch (const filesystem::filesystem_error& e) {
        umask(oldMask);
        debug::log(debug::LogTypeError, "Failed to create " + tempDirPath);
        return;
    }
    umask(oldMask);

    // Tighten directory permissions in case it already existed with looser umask permissions
    chmod(tempDirPath.c_str(), S_IRWXU);

    string tempAuthInfoPath = settings::joinAppDataPath("/.tmp/auth_info.json");
    string payload = __makeAuthInfoPayload().dump();

    // Create or truncate the file atomically with S_IRUSR | S_IWUSR (0600)
    int fd = open(tempAuthInfoPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if(fd == -1) {
        debug::log(debug::LogTypeError, "Failed to create auth info file: " + tempAuthInfoPath);
        return;
    }

    // Enforce 0600 in case the file already existed with looser permissions
    fchmod(fd, S_IRUSR | S_IWUSR);

    ssize_t bytesWritten = write(fd, payload.c_str(), payload.size());
    close(fd);

    if(bytesWritten < 0 || (size_t)bytesWritten != payload.size()) {
        debug::log(debug::LogTypeError, "Failed to write complete auth info to " + tempAuthInfoPath);
        return;
    }
#else
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

#if defined(_WIN32)
    error_code ec;
    filesystem::permissions(CONVSTR(tempDirPath), filesystem::perms::owner_all, filesystem::perm_options::replace, ec);
    filesystem::permissions(CONVSTR(tempAuthInfoPath), filesystem::perms::owner_read | filesystem::perms::owner_write, filesystem::perm_options::replace, ec);
#endif

#endif

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
