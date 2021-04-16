#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <vector>
#include "helpers.h"
#include "settings.h"
#include "lib/json.hpp"
#include "auth/authbasic.h"
#include "permission.h"

#if defined(__linux__)
#include "../core-linux/src/api/filesystem/filesystem.h"
#include "../core-linux/src/api/os/os.h"
#include "../core-linux/src/api/computer/computer.h"
#include "../core-linux/src/api/storage/storage.h"
#include "../core-linux/src/api/debug/debug.h"
#include "../core-linux/src/api/app/app.h"

#elif defined(_WIN32)
#include "../core-windows/src/api/filesystem/filesystem.h"
#include "../core-windows/src/api/os/os.h"
#include "../core-windows/src/api/computer/computer.h"
#include "../core-windows/src/api/storage/storage.h"
#include "../core-windows/src/api/debug/debug.h"
#include "../core-windows/src/api/app/app.h"
#endif

using namespace std;
using json = nlohmann::json;
typedef string (*NativeMethod)(json);

namespace routes {

    pair<string, string> executeNativeMethod(string path, string postData, string token) {
        string modfunc = regex_replace(path, std::regex("/__nativeMethod_"), "");
        string output = "";

        if(!authbasic::verifyToken(token))
            return make_pair("{\"error\":\"Invalid or expired NL_TOKEN value from client\"}", "application/json");
        if(!permission::hasAccess(modfunc))
            return make_pair("{\"error\":\"Missing permission to execute the native method\"}", "application/json");

        map <string, NativeMethod> methodMap = {
            {"app.exit", app::exit},
            {"app.getConfig", app::getConfig},
            {"app.keepAlive", app::keepAlive},
            {"app.open", app::open},
            {"computer.getRamUsage", computer::getRamUsage},
            {"debug.log", debug::log},
            {"filesystem.createDirectory", fs::createDirectory},
            {"filesystem.removeDirectory", fs::removeDirectory},
            {"filesystem.readFile", fs::readFile},
            {"filesystem.writeFile", fs::writeFile},
            {"filesystem.removeFile", fs::removeFile},
            {"filesystem.readDirectory", fs::readDirectory},
            {"os.execCommand", os::execCommand},
            {"os.getEnvar", os::getEnvar},
            {"os.dialogOpen", os::dialogOpen},
            {"os.dialogSave", os::dialogSave},
            {"os.showNotification", os::showNotification},
            {"os.showMessageBox", os::showMessageBox},
            {"storage.putData", storage::putData},
            {"storage.getData", storage::getData}
        };

        if(methodMap.find(modfunc) != methodMap.end() ){
            json inputPayload;
            try {
                if(postData != "")
                    inputPayload = json::parse(postData);
                NativeMethod nativeMethod = methodMap[modfunc];
                output = (*nativeMethod)(inputPayload);
            }
            catch(exception e){
                json parserOutput = {{"error", "JSON parse error. Please check whether your request payload is correct"}};
                output = parserOutput.dump();
            }
        }
        else {
            json defaultOutput = {{"error", modfunc + " is not implemented in the Neutralinojs server"}};
            output = defaultOutput.dump();
        }

        return make_pair(output, "application/json");
    }

    pair<string, string> getAsset(string path, string prependData = "") {
        vector<string> split = helpers::split(path, '.');
        if(split.size() < 2) {
            if(path.back() != '/')
                path += "/";
            return getAsset(path + "index.html", prependData);
        }
        string extension = split[split.size() - 1];
        map<string, string> mimeTypes = {
            {"js", "text/javascript"},
            {"css", "text/css"},
            {"html", "text/html"},
            {"jpg", "image/jpeg"},
            {"png", "image/png"},
            {"svg", "image/svg+xml"},
            {"gif", "image/gif"},
            {"ico", "image/x-icon"},
            {"woff2", "font/woff2"},
            {"mp3", "audio/mpeg"},
            {"xml", "application/xml"},
            {"json", "application/json"}
        };
        string fileData = settings::getFileContent(path);
        if(prependData != "")
            fileData = prependData + fileData;
        return make_pair(fileData, mimeTypes[extension]);
    }

   pair<string, string> handle(string encodedPath, string postData, string token) {
        char *originalPath = (char *) encodedPath.c_str();
        char *decodedPath = new char[strlen(originalPath) + 1];
        helpers::urldecode(decodedPath, originalPath);
        string path = string(decodedPath);
        delete []decodedPath;

        bool isNativeMethod = regex_match(path, regex(".*/__nativeMethod_.*"));
        bool isClientLibrary = regex_match(path, regex(".*neutralino.js$"));

        if(isNativeMethod) {
            return executeNativeMethod(path, postData, token);
        }
        else if(isClientLibrary) {
            return getAsset(path, settings::getGlobalVars());
        }
        else {
            return getAsset(path);
        }
    }

}
