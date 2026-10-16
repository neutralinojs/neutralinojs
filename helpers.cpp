#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <ctype.h>
#include <random>
#include <optional>
#include <regex>
#include <filesystem>

#include "helpers.h"
#include "lib/json/json.hpp"

#if defined(_WIN32)
#include <string>
#include <windows.h>
#endif

using namespace std;
using json = nlohmann::json;

namespace helpers {

vector<string> split(const string &s, char delim, unsigned int stopAfter) {
    stringstream ss(s);
    string item;
    vector<string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
        if(stopAfter !=-1 && tokens.size() == stopAfter - 1) {
            delim = '\n';
        }
    }
    return tokens;
}

vector<string> splitTwo(const string &s, char delim) {
    return split(s, delim, 2);
}

string trimRight(string &s) {
    s.erase(find_if(s.rbegin(), s.rend(),
        [](unsigned char c) {
            return !isspace(c);
    }).base(), s.end());
    return s;
}

string generateToken() {
    static const string characters =
        "-_"
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    const int length = 96;

    string token;
    token.reserve(length);

    random_device rd;
    mt19937 generator(rd()); 
    uniform_int_distribution<int> distribution(0, characters.size() - 1);

    for (int i = 0; i < length; ++i) {
        if (i == 47) {
            token += ".";
        }
        token += characters[distribution(generator)];
    }
    
    return token;
}

/*
* https://stackoverflow.com/a/14530993 - mini url decoder
*/
void urldecode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a')
                a -= 'a' - 'A';
            if (a >= 'A')
                a -= ('A' - 10);
            else
                a -= '0';
            if (b >= 'a')
                b -= 'a' - 'A';
            if (b >= 'A')
                b -= ('A' - 10);
            else
                b -= '0';
            *dst++ = 16 * a + b;
            src += 3;
        }
        else {
            *dst++ = *src++;
        }
    }
    *dst++ = '\0';
}

char* cStrCopy(const string &str) {
    char *text = new char[str.size() + 1];
    copy(str.begin(), str.end(), text);
    text[str.size()] = '\0';
    // delete[] text from the initiator
    return text;
}

bool hasRequiredFields(const json &input, const vector<string> &keys) {
    for(const string &key: keys) {
        if(!helpers::hasField(input, key)) {
            return false;
        }
    }
    return true;
}

optional<string> missingRequiredField(const json &input, const vector<string> &keys) {
    for(const string &key: keys) {
        if(!helpers::hasField(input, key)) {
            return key;
        }
    }
    return {};
}

bool hasField(const json &input, const string &key) {
    return input.contains(key) && !input[key].is_null();
}

vector<string> getModes() {
    return {"window", "browser", "cloud", "chrome"};
}

string appModeToStr(settings::AppMode mode) {
    switch(mode) {
        case settings::AppModeWindow:
            return "window";
        case settings::AppModeBrowser:
            return "browser";
        case settings::AppModeCloud:
            return "cloud";
        case settings::AppModeChrome:
            return "chrome";
        default:
            return "invalid";
    }
}

string getCurrentTimestamp() {
    auto now = chrono::system_clock::now();
    auto nowTimeT = chrono::system_clock::to_time_t(now);
    auto nowMs = chrono::duration_cast<chrono::milliseconds>(
                      now.time_since_epoch())
                      .count() % 1000;

    ostringstream oss;
    oss << put_time(gmtime(&nowTimeT), "%Y-%m-%dT%H:%M:%S")
        << "." << setfill('0') << setw(3) << nowMs << "Z";

    return oss.str();
}

string normalizePath(string &path) {
    #if defined(_WIN32)
    replace(path.begin(), path.end(), '\\', '/');
    #endif
    if(path.size() > 1 && path.back() == '/') {
        path.pop_back();
    }
    return path;
}

string unNormalizePath(string &path) {
    #if defined(_WIN32)
    replace(path.begin(), path.end(), '/', '\\');
    #endif
    return path;
}

string jsonToString(const json &obj) {
    return obj.dump(-1, ' ', false, json::error_handler_t::replace);
}

#if defined(_WIN32)
wstring str2wstr(const string &str) {
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    wstring ret(len, '\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), (LPWSTR)ret.data(), (int)ret.size());
    return ret;
}

string wstr2str(const wstring &str) {
    int len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0, nullptr, nullptr);
    string ret(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(), (LPSTR)ret.data(), (int)ret.size(), nullptr, nullptr);
    return ret;
}

string wcstr2str(const wchar_t* wstr) {
    int count = WideCharToMultiByte(CP_UTF8, 0, wstr, wcslen(wstr), NULL, 0, NULL, NULL);
    string str(count, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], count, NULL, NULL);
    return str;
}
#endif

bool globalMatch(const string &value, const string &pattern) {
    string regexStr = "^";
    for(const char c: pattern) {
        switch(c) {
            case '*': regexStr += ".*"; break;
            case '?': regexStr += "."; break;
            case '.': case '\\': case '+': case '(': case ')':
            case '[': case ']': case '{': case '}': case '|':
            case '^': case '$': case '#': regexStr += "\\"; [[fallthrough]];
            default: regexStr += c; break;
        }
    }
    regexStr += "$";
    try {
        return regex_match(value, regex(regexStr));
    }
    catch(const regex_error &) {
        return false;
    }
}

vector<string> tokenizeCommand(const string &command) {
    vector<string> tokens;
    string current;
    bool inSingle = false;
    bool inDouble = false;

    auto flushToken = [&](void) {
        if(!current.empty()) {
            tokens.push_back(current);
            current.clear();
        }
    };

    for(size_t i = 0; i < command.size(); i++) {
        unsigned char c = static_cast<unsigned char>(command[i]);
        if(!inSingle && !inDouble) {
            if(isspace(c)) {
                flushToken();
            }
            else if(c == '\'') {
                inSingle = true;
            }
            else if(c == '"') {
                inDouble = true;
            }
            else if(strchr(";|&><$`(){}\\\n\r", c) != nullptr) {
                tokens.clear();
                return tokens;
            }
            else {
                current += static_cast<char>(c);
            }
        }
        else if(inSingle) {
            if(c == '\'') {
                inSingle = false;
            }
            else {
                current += static_cast<char>(c);
            }
        }
        else {
            if(c == '"') {
                inDouble = false;
            }
            else {
                current += static_cast<char>(c);
            }
        }
    }
    if(inSingle || inDouble) {
        tokens.clear();
        return tokens;
    }
    flushToken();
    return tokens;
}

/**
 * Checks whether targetPath is strictly inside (or equal to) baseDirectory after
 * resolving relative segments, canonicalizing symlinks, and normalizing path syntax.
 *
 * Security Rationale:
 * - GHSA-645w-p5gr-j5xm (CWE-22): Path traversal via `..` and encoded `%2e%2e` in static HTTP server.
 * - GHSA-vc3q-6mj2-qfg6 (CWE-59): Path traversal via symbolic links pointing outside app directories.
 * - GHSA-x53c-rxmx-mg69 (CWE-22): Path traversal in endpoints mounted via `server.mount`.
 *
 * Design & Architecture:
 * Rather than scattering ad-hoc lexical string checks (which are prone to bypasses such as
 * mixed forward/backward slashes, case normalization quirks, or symlinks on disk), this function
 * establishes a single, robust abstraction boundary using C++17 `std::filesystem`.
 *
 * Why `std::filesystem::weakly_canonical`?
 * 1. Standard `canonical()` throws if the target path does not exist on disk.
 *    `weakly_canonical()` resolves all existing parent components (including symlinks and `..` segments)
 *    and normalizes any non-existent tail components without throwing an exception.
 * 2. It resolves relative `.` and `..` components cleanly across all platforms.
 * 3. It unwinds symlinks up to the existing filesystem node, ensuring a symlink inside baseDirectory
 *    cannot secretly resolve to a location outside baseDirectory.
 */
bool isPathInsideDirectory(const string &targetPath, const string &baseDirectory) {
    if (baseDirectory.empty() || targetPath.empty()) {
        return false;
    }

    error_code ecBase, ecTarget;

    // 1. Resolve base directory to its canonical form so we have a reliable reference root.
    filesystem::path base = filesystem::weakly_canonical(filesystem::path(CONVSTR(baseDirectory)), ecBase);
    if (ecBase) {
        // If the base directory path is invalid or inaccessible, deny access by default.
        return false;
    }

    // 2. Resolve the target path (canonicalizing all symlinks and collapsing '..' segments).
    filesystem::path target = filesystem::weakly_canonical(filesystem::path(CONVSTR(targetPath)), ecTarget);
    if (ecTarget) {
        // Failed to resolve target path (e.g. permission or platform filesystem error).
        return false;
    }

    // 3. Convert both paths to unified string representations and normalize slashes.
    // On Windows, FS_CONVWSTRN / helpers::normalizePath convert backslashes to forward slashes.
    string normBase = FS_CONVWSTRN(base);
    string normTarget = FS_CONVWSTRN(target);

    helpers::normalizePath(normBase);
    helpers::normalizePath(normTarget);

    // 4. Exact match: target is the base directory itself.
    if (normTarget == normBase) {
        return true;
    }

    // 5. Prefix containment check:
    // Ensure target begins with normBase followed immediately by a directory separator ('/').
    // If normBase already ends with '/' (such as root "/"), the prefix match already covers the boundary.
    // This prevents false positives where base is "/app" and target is "/app-leak/secret.txt".
    if (normTarget.size() > normBase.size() &&
        normTarget.compare(0, normBase.size(), normBase) == 0 &&
        (normBase.back() == '/' || normTarget[normBase.size()] == '/')) {
        return true;
    }

    // The target path resolves outside baseDirectory.
    return false;
}

} // namespace helpers
