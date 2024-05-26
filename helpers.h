#ifndef NEU_HELPERS_H
#define NEU_HELPERS_H

#include <vector>
#include <string>

#include "lib/json/json.hpp"

#include "settings.h"

using namespace std;
using json = nlohmann::json;

namespace helpers {

vector<string> split(const string &s, char delim);
string generateToken();
void urldecode(char *dst, const char *src);
char* cStrCopy(const string &str);
bool hasRequiredFields(const json &input, const vector<string> &keys);
bool hasField(const json &input, const string &key);
vector<string> getModes();
string appModeToStr(settings::AppMode mode);
string normalizePath(string &path);
string unNormalizePath(string &path);

#if defined(_WIN32)
wstring str2wstr(const string &str);
string wstr2str(const wstring &str);
string wcstr2str(const wchar_t* wstr);
#endif

} // namespace helpers

#endif // #define NEU_HELPERS_H


