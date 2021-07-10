#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <vector>
#include "helpers.h"
#include "settings.h"
#include "lib/json.hpp"
#include "auth/authbasic.h"
#include "auth/permission.h"
#include "api/filesystem/filesystem.h"
#include "api/os/os.h"
#include "api/computer/computer.h"
#include "api/storage/storage.h"
#include "api/debug/debug.h"
#include "api/app/app.h"
#include "api/window/window.h"

#if defined(__APPLE__)
#include <dispatch/dispatch.h>
#endif

using namespace std;
using json = nlohmann::json;
typedef json (*NativeMethod)(json);

namespace routes {
    pair<string, string> executeNativeMethod(string path, string postData, string token) {
        string modfunc = regex_replace(path, std::regex("/__nativeMethod_"), "");

        #if (__APPLE__)
        __block string output = "";
        __block json inputPayload;
        #else
        string output = "";
        json inputPayload;
        #endif

        if(!authbasic::verifyToken(token))
            return make_pair("{\"error\":\"Invalid or expired NL_TOKEN value from client\"}", "application/json");
        if(!permission::hasAccess(modfunc))
            return make_pair("{\"error\":\"Missing permission to execute the native method\"}", "application/json");
        if(!permission::hasAPIAccess(modfunc) && modfunc != "app.keepAlive")
            return make_pair("{\"error\":\"Missing permission to access Native API\"}", "application/json");

        map <string, NativeMethod> methodMap = {
            {"app.exit", app::controllers::exit},
            {"app.getConfig", app::controllers::getConfig},
            {"app.keepAlive", app::controllers::keepAlive},
            {"app.open", app::controllers::open},
            {"window.setTitle", window::controllers::setTitle},
            {"window.maximize", window::controllers::maximize},
            {"window.isMaximized", window::controllers::isMaximized},
            {"window.unmaximize", window::controllers::unmaximize},
            {"window.minimize", window::controllers::minimize},
            {"computer.getRamUsage", computer::controllers::getRamUsage},
            {"debug.log", debug::controllers::log},
            {"filesystem.createDirectory", fs::controllers::createDirectory},
            {"filesystem.removeDirectory", fs::controllers::removeDirectory},
            {"filesystem.readFile", fs::controllers::readFile},
            {"filesystem.writeFile", fs::controllers::writeFile},
            {"filesystem.removeFile", fs::controllers::removeFile},
            {"filesystem.readDirectory", fs::controllers::readDirectory},
            {"os.execCommand", os::controllers::execCommand},
            {"os.getEnvar", os::controllers::getEnvar},
            {"os.dialogOpen", os::controllers::dialogOpen},
            {"os.dialogSave", os::controllers::dialogSave},
            {"os.showNotification", os::controllers::showNotification},
            {"os.showMessageBox", os::controllers::showMessageBox},
            {"os.setTray", os::controllers::setTray},
            {"storage.putData", storage::controllers::putData},
            {"storage.getData", storage::controllers::getData}
        };

        if(methodMap.find(modfunc) != methodMap.end() ){
            try {
                if(postData != "")
                    inputPayload = json::parse(postData);
                NativeMethod nativeMethod = methodMap[modfunc];
                json apiOutput;
                #if defined(__APPLE__)
                // In macos, child threads cannot run UI logic
                if(modfunc == "os.showMessageBox" || 
                    regex_match(modfunc, regex("^window.*")) || 
                    modfunc == "os.setTray") {
                    dispatch_sync(dispatch_get_main_queue(), ^{
                        apiOutput = (*nativeMethod)(inputPayload);
                    });
                }
                else {
                    apiOutput = (*nativeMethod)(inputPayload);
                }
                #else
                    apiOutput = (*nativeMethod)(inputPayload);
                #endif
                output = apiOutput.dump();
            }
            catch(exception e){
                json parserOutput = {{"error", "Native method execution error occured"}};
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
