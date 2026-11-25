#include <string>
#include <filesystem>

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#if defined(_WIN32)
#define _WINSOCKAPI_
#include <windows.h>
#include <sddl.h>
#include <aclapi.h>
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

json makeAuthInfoPayload() {
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
    string payload = makeAuthInfoPayload().dump();

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
#elif defined(_WIN32)
    // On Windows NTFS, std::filesystem::permissions only modifies the FILE_ATTRIBUTE_READONLY
    // flag and does NOT alter the Discretionary Access Control List (DACL). Default inherited
    // permissions grant BUILTIN\Users read access.
    // We explicitly set a protected DACL restricting access to Object Owner (OW),
    // BUILTIN\Administrators (BA), and NT AUTHORITY\SYSTEM (SY).
    auto setWindowsAcl = [](const wstring &path, LPCWSTR sddl, HANDLE hExistingFile = INVALID_HANDLE_VALUE) {
        PSECURITY_DESCRIPTOR pSD = nullptr;
        if(!ConvertStringSecurityDescriptorToSecurityDescriptorW(sddl, SDDL_REVISION_1, &pSD, nullptr)) {
            return;
        }
        PACL pAcl = nullptr;
        BOOL daclPresent = FALSE, daclDefaulted = FALSE;
        if(GetSecurityDescriptorDacl(pSD, &daclPresent, &pAcl, &daclDefaulted)) {
            if(hExistingFile != INVALID_HANDLE_VALUE) {
                SetSecurityInfo(hExistingFile, SE_FILE_OBJECT,
                    DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
                    nullptr, nullptr, pAcl, nullptr);
            }
            SetNamedSecurityInfoW(const_cast<LPWSTR>(path.c_str()), SE_FILE_OBJECT,
                DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
                nullptr, nullptr, pAcl, nullptr);
        }
        LocalFree(pSD);
    };

    wstring wTempDirPath = CONVSTR(tempDirPath);
    string tempAuthInfoPath = settings::joinAppDataPath("/.tmp/auth_info.json");
    wstring wTempAuthInfoPath = CONVSTR(tempAuthInfoPath);

    LPCWSTR dirSddl = L"D:P(A;OICI;GA;;;OW)(A;OICI;GA;;;BA)(A;OICI;GA;;;SY)";
    PSECURITY_DESCRIPTOR pDirSD = nullptr;
    SECURITY_ATTRIBUTES dirSA = { sizeof(SECURITY_ATTRIBUTES), nullptr, FALSE };
    if(ConvertStringSecurityDescriptorToSecurityDescriptorW(dirSddl, SDDL_REVISION_1, &pDirSD, nullptr)) {
        dirSA.lpSecurityDescriptor = pDirSD;
    }

    if(!CreateDirectoryW(wTempDirPath.c_str(), pDirSD ? &dirSA : nullptr)) {
        if(GetLastError() != ERROR_ALREADY_EXISTS) {
            error_code ec;
            filesystem::create_directories(CONVSTR(tempDirPath), ec);
        }
    }
    setWindowsAcl(wTempDirPath, dirSddl);
    if(pDirSD) {
        LocalFree(pDirSD);
    }

    LPCWSTR fileSddl = L"D:P(A;;GA;;;OW)(A;;GA;;;BA)(A;;GA;;;SY)";
    PSECURITY_DESCRIPTOR pFileSD = nullptr;
    SECURITY_ATTRIBUTES fileSA = { sizeof(SECURITY_ATTRIBUTES), nullptr, FALSE };
    if(ConvertStringSecurityDescriptorToSecurityDescriptorW(fileSddl, SDDL_REVISION_1, &pFileSD, nullptr)) {
        fileSA.lpSecurityDescriptor = pFileSD;
    }

    HANDLE hFile = CreateFileW(wTempAuthInfoPath.c_str(), GENERIC_WRITE, 0,
                               pFileSD ? &fileSA : nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if(hFile == INVALID_HANDLE_VALUE) {
        if(pFileSD) LocalFree(pFileSD);
        debug::log(debug::LogTypeError, "Failed to create auth info file: " + tempAuthInfoPath);
        return;
    }

    setWindowsAcl(wTempAuthInfoPath, fileSddl, hFile);
    if(pFileSD) {
        LocalFree(pFileSD);
    }

    string payload = makeAuthInfoPayload().dump();
    DWORD bytesWritten = 0;
    BOOL writeSuccess = WriteFile(hFile, payload.c_str(), static_cast<DWORD>(payload.size()), &bytesWritten, nullptr);
    CloseHandle(hFile);

    if(!writeSuccess || bytesWritten != payload.size()) {
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
        makeAuthInfoPayload().dump()
    };
    fs::writeFile(fileWriterOptions);
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
