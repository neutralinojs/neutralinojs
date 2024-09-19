#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <time.h>
#include <ctype.h>

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

string generateToken() {
    srand(time(NULL));

    string s = "";
    static const char alphanum[] =
        "-_"
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < 96; ++i) {
        s += alphanum[rand() % (sizeof(alphanum) - 1)];
        if(i == 47) {
            s += ".";
        }
    }

    return s;
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
        else if (*src == '+') {
            *dst++ = ' ';
            src++;
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

} // namespace helpers
