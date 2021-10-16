#include <string>

#include <websocketpp/server.hpp>


#include "lib/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace router {
    
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
    router::NativeMessage executeNativeMethod(router::NativeMessage request);
    router::Response makeNativeResponse(string data);
    router::Response makeNativeFailResponse(string errorCode, string errorMessage);
    router::Response getAsset(string path, string prependData = "");
}
