#include <string>

using namespace std;

namespace routes {
    pair<string, string> handle(string path, string j, string token);
    pair<string, string> executeNativeMethod(string path, string postData, string token);
    pair<string, string> getAsset(string path, string prependData = "");
}
