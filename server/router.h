#include <string>

using namespace std;

namespace router {
    
    struct Response {
        string header;
        string data;
    };
    
    struct Request {
        string path;
        string data;
        string token;
    };

    router::Response handle(router::Request request);
    router::Response executeNativeMethod(router::Request request);
    router::Response makeNativeResponse(string data);
    router::Response makeNativeFailResponse(string errorCode, string errorMessage);
    router::Response getAsset(string path, string prependData = "");
}
