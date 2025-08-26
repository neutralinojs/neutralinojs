#ifndef NEU_HELPERS_H
#define NEU_HELPERS_H

#include <vector>
#include <string>
#include <optional>

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#define CONVSTR(S) S
#define FS_CONVWSTR(S) S.string()
#define FS_CONVWSTRN(S) S.string()
#elif defined(_WIN32)
#define CONVSTR(S) helpers::str2wstr(S)
#define FS_CONVWSTR(S) helpers::wstr2str(S.wstring())
#define FS_CONVWSTRN(S) helpers::normalizePath(helpers::wstr2str(S.wstring()))
#endif

#include "lib/json/json.hpp"

#include "settings.h"

using namespace std;
using json = nlohmann::json;

namespace helpers {

vector<string> split(const string &s, char delim, unsigned int stopAfter = -1);
vector<string> splitTwo(const string &s, char delim);
string generateToken();
void urldecode(char *dst, const char *src);
char* cStrCopy(const string &str);
bool hasRequiredFields(const json &input, const vector<string> &keys);
optional<string> missingRequiredField(const json &input, const vector<string> &keys);
bool hasField(const json &input, const string &key);
vector<string> getModes();
string appModeToStr(settings::AppMode mode);
string normalizePath(string &path);
string unNormalizePath(string &path);
string getCurrentTimestamp();
string jsonToString(const json &obj);

#if defined(_WIN32)
wstring str2wstr(const string &str);
string wstr2str(const wstring &str);
string wcstr2str(const wchar_t* wstr);
#endif

} // namespace helpers

#endif // #define NEU_HELPERS_H


