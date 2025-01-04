#ifndef NEU_ROUTER_H
#define NEU_ROUTER_H

#include <string>

#include <websocketpp/server.hpp>

#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace router {

void mountPath(const string &path, const string &mountPoint);
bool isMounted(const string &path);
void unmountPath(const string &path);

typedef json (*NativeMethod)(const json &);

struct Response {
    websocketpp::http::status_code::value status = websocketpp::http::status_code::ok;
    string contentType = "application/octet-stream";
    string data;
};

struct NativeMessage {
    string id;
    string method;
    string accessToken;
    json data;
};

router::Response serve(string path);
router::NativeMessage executeNativeMethod(const router::NativeMessage &request);
router::Response getAsset(string path, const string &prependData = "");
map<string, router::NativeMethod> getMethodMap();

} // namespace router

#endif // #define NEU_ROUTER_H
