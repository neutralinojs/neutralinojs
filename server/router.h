#ifndef NEU_ROUTER_H
#define NEU_ROUTER_H

#include <string>
#include <map>

#include <websocketpp/server.hpp>

#include "lib/json/json.hpp"
#include "errors.h"
#include "api/fs/fs.h"

using namespace std;
using json = nlohmann::json;

namespace router {

typedef json (*NativeMethod)(const json &);

struct Response {
    websocketpp::http::status_code::value status = websocketpp::http::status_code::ok;
    string contentType = "application/octet-stream";
    string data;
    map<string, string> headers;
};

struct NativeMessage {
    string id;
    string method;
    string accessToken;
    json data;
};

router::Response serve(string path, const fs::FileReaderOptions &fileReaderOptions = {});
router::NativeMessage executeNativeMethod(const router::NativeMessage &request);
router::Response getAsset(string path, const string &prependData = "", const fs::FileReaderOptions &fileReaderOptions = {});
map<string, router::NativeMethod> getMethodMap();
errors::StatusCode mountPath(string &path, string &target);
bool isMounted(const string &path);
bool unmountPath(string &path);
map<string, string> getMounts();

} // namespace router

#endif // #define NEU_ROUTER_H
