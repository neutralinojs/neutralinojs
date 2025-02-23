#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <ctype.h>
#include <random>

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

string sanitizeUTF8(const std::string &input)
{
    string sanitized;
    sanitized.reserve(input.size());
    auto isContinuationByte = [](uint8_t byte)
    {
        return (byte & 0xC0) == 0x80;
    };
    for (size_t i = 0; i < input.size();)
    {
        uint8_t byte = static_cast<uint8_t>(input[i]);
        if (byte <= 0x7F)
        {
            sanitized.push_back(byte);
            i++;
        }
        else if ((byte & 0xE0) == 0xC0)
        {
            if (i + 1 < input.size() && isContinuationByte(input[i + 1]))
            {
                sanitized.push_back(byte);sanitized.push_back(input[i + 1]);
                i += 2;
            }
            else
            {
                sanitized.push_back('?');i++;
            }
        }
        else if ((byte & 0xF0) == 0xE0)
        {
            if (i + 2 < input.size() && isContinuationByte(input[i + 1]) && isContinuationByte(input[i + 2]))
            {
                sanitized.push_back(byte);sanitized.push_back(input[i + 1]);
                sanitized.push_back(input[i + 2]);i += 3;
            }
            else
            {
                sanitized.push_back('?');
                i++;
            }
        }
        else if ((byte & 0xF8) == 0xF0)
        {
            if (i + 3 < input.size() && isContinuationByte(input[i + 1]) && isContinuationByte(input[i + 2]) && isContinuationByte(input[i + 3]))
            {
                sanitized.push_back(byte);sanitized.push_back(input[i + 1]);
                sanitized.push_back(input[i + 2]);sanitized.push_back(input[i + 3]);
                i += 4;
            }
            else
            {
                sanitized.push_back('?');i++;
            }
        }
        else
        {
            sanitized.push_back('?');i++;
        }
    }
    return sanitized;
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
