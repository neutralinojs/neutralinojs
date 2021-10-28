#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <vector>

#include <websocketpp/server.hpp>

#include "lib/json/json.hpp"
#include "auth/authbasic.h"
#include "auth/permission.h"
#include "server/router.h"
#include "helpers.h"
#include "settings.h"
#include "api/os/os.h"
#include "api/filesystem/filesystem.h"
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
typedef json (*NativeMethod)(const json &);

namespace router {
    router::NativeMessage executeNativeMethod(const router::NativeMessage &request) {
        string nativeMethodId = request.method;
        router::NativeMessage response;
        response.id = request.id;
        response.method = request.method;

        if(!authbasic::verifyToken(request.accessToken)) {
            response.data = helpers::makeErrorPayload("NE_RT_INVTOKN",
                            "Invalid or expired NL_TOKEN value from client").dump();
            return response;
        }
        if(!permission::hasMethodAccess(nativeMethodId)) {
            response.data = helpers::makeErrorPayload("NE_RT_NATPRME", 
                            "Missing permission to execute the native method").dump();
            return response;
        }
        if(!permission::hasAPIAccess(nativeMethodId)) {
            response.data = helpers::makeErrorPayload("NE_RT_APIPRME",
                            "Missing permission to access Native API").dump();
            return response;
        }

        map <string, NativeMethod> methodMap = {
            // Neutralino.app
            {"app.exit", app::controllers::exit},
            {"app.killProcess", app::controllers::killProcess},
            {"app.getConfig", app::controllers::getConfig},
            {"app.keepAlive", app::controllers::keepAlive},
            // Neutralino.window
            {"window.setTitle", window::controllers::setTitle},
            {"window.maximize", window::controllers::maximize},
            {"window.isMaximized", window::controllers::isMaximized},
            {"window.unmaximize", window::controllers::unmaximize},
            {"window.minimize", window::controllers::minimize},
            {"window.isVisible", window::controllers::isVisible},
            {"window.show", window::controllers::show},
            {"window.hide", window::controllers::hide},
            {"window.isFullScreen", window::controllers::isFullScreen},
            {"window.setFullScreen", window::controllers::setFullScreen},
            {"window.exitFullScreen", window::controllers::exitFullScreen},
            {"window.focus", window::controllers::focus},
            {"window.setIcon", window::controllers::setIcon},
            {"window.move", window::controllers::move},
            {"window.setSize", window::controllers::setSize},
            // Neutralino.computer
            {"computer.getMemoryInfo", computer::controllers::getMemoryInfo},
            // Neutralino.log
            {"debug.log", debug::controllers::log},
            // Neutralino.filesystem
            {"filesystem.createDirectory", fs::controllers::createDirectory},
            {"filesystem.removeDirectory", fs::controllers::removeDirectory},
            {"filesystem.readFile", fs::controllers::readFile},
            {"filesystem.readBinaryFile", fs::controllers::readBinaryFile},
            {"filesystem.writeFile", fs::controllers::writeFile},
            {"filesystem.writeBinaryFile", fs::controllers::writeBinaryFile},
            {"filesystem.removeFile", fs::controllers::removeFile},
            {"filesystem.readDirectory", fs::controllers::readDirectory},
            {"filesystem.copyFile", fs::controllers::copyFile},
            {"filesystem.moveFile", fs::controllers::moveFile},
            {"filesystem.getStats", fs::controllers::getStats},
            // Neutralino.os
            {"os.execCommand", os::controllers::execCommand},
            {"os.getEnv", os::controllers::getEnv},
            {"os.showOpenDialog", os::controllers::showOpenDialog},
            {"os.showFolderDialog", os::controllers::showFolderDialog},
            {"os.showSaveDialog", os::controllers::showSaveDialog},
            {"os.showNotification", os::controllers::showNotification},
            {"os.showMessageBox", os::controllers::showMessageBox},
            {"os.setTray", os::controllers::setTray},
            {"os.open", os::controllers::open},
            {"os.getPath", os::controllers::getPath},
            // Neutralino.storage
            {"storage.setData", storage::controllers::setData},
            {"storage.getData", storage::controllers::getData}
        };

        if(methodMap.find(nativeMethodId) != methodMap.end()) {
            try {
                NativeMethod nativeMethod = methodMap[nativeMethodId];
                #if defined(__linux__) || defined(_WIN32) || defined(__FreeBSD__)
                json apiOutput;
                #endif
                #if defined(__APPLE__)
                __block json apiOutput;
                // In macos, child threads cannot run UI logic
                if(nativeMethodId == "os.showMessageBox" || 
                    regex_match(nativeMethodId, regex("^window.*")) || 
                    nativeMethodId == "os.setTray") {
                    dispatch_sync(dispatch_get_main_queue(), ^{
                        apiOutput = (*nativeMethod)(request.data);
                    });
                }
                else {
                    apiOutput = (*nativeMethod)(request.data);
                }
                #else
                    apiOutput = (*nativeMethod)(request.data);
                #endif
                response.data = apiOutput;
                return response;
            }
            catch(exception e){
                response.data["error"] = helpers::makeErrorPayload("NE_RT_NATRTER", 
                        "Native method execution error occurred. Failed because of: " + std::string(e.what()) + 
                        ". Make sure that you've provided required parameters properly.");
                return response;
            }
        }
        else {
            response.data["error"] = helpers::makeErrorPayload("NE_RT_NATNTIM",
                        nativeMethodId + " is not implemented in the Neutralinojs server");
            return response;
        }
    }

    router::Response getAsset(string path, const string &prependData) {
        router::Response response;
        vector<string> split = helpers::split(path, '.');
        if(split.size() < 2) {
            if(path.back() != '/')
                path += "/";
            return getAsset(path + "index.html", prependData);
        }
        string extension = split[split.size() - 1];
        map<string, string> mimeTypes = {
            // Plain text files
            {"css", "text/css"},
            {"csv", "text/csv"},
            {"txt", "text/plain"},
            {"vtt", "text/vtt"},
            {"htm", "text/html"},
            {"html", "text/html"},   
            // Image files
            {"apng", "image/apng"},
            {"avif", "image/avif"},
            {"bmp", "image/bmp"},
            {"gif", "image/gif"},
            {"png", "image/png"},
            {"svg", "image/svg+xml"},
            {"webp", "image/webp"},
            {"ico", "image/x-icon"},
            {"tif", "image/tiff"},
            {"tiff", "image/tiff"},
            {"jpg", "image/jpeg"},
            {"jpeg", "image/jpeg"}, 
            // Video files
            {"mp4", "video/mp4"},
            {"mpeg", "video/mpeg"},
            {"webm", "video/webm"},
            // Audio files
            {"mp3", "audio/mp3"},
            {"mpga", "audio/mpeg"},
            {"weba", "audio/webm"},
            {"wav", "audio/wave"},
            // Font files
            {"otf", "font/otf"},
            {"ttf", "font/ttf"},
            {"woff", "font/woff"},
            {"woff2", "font/woff2"},
            // Application-type files
            {"7z", "application/x-7z-compressed"},
            {"atom", "application/atom+xml"},
            {"pdf", "application/pdf"},
            {"js", "application/javascript"},
            {"mjs", "application/javascript"},
            {"json", "application/json"},
            {"rss", "application/rss+xml"},
            {"tar", "application/x-tar"},
            {"xht", "application/xhtml+xml"},
            {"xhtml", "application/xhtml+xml"},
            {"xslt", "application/xslt+xml"},
            {"xml", "application/xml"},
            {"gz", "application/gzip"},
            {"zip", "application/zip"},
            {"wasm", "application/wasm"}
        };

        fs::FileReaderResult fileReaderResult = settings::getFileContent(path);
        response.data = fileReaderResult.data;
        response.status = fileReaderResult.hasError ? websocketpp::http::status_code::not_found
                                    : websocketpp::http::status_code::ok;
        if(prependData != "")
            response.data = prependData + response.data;
        
        // If MIME-type is not defined in neuserver, application/octet-stream will be used by default.
        if(mimeTypes.find(extension) != mimeTypes.end()) {
            response.contentType = mimeTypes[extension];
        }
        return response;
    }

   router::Response serve(string path) {
        char *originalPath = (char *) path.c_str();
        char *decodedPath = new char[strlen(originalPath) + 1];
        helpers::urldecode(decodedPath, originalPath);
        path = string(decodedPath);
        delete []decodedPath;

        bool isClientLibrary = regex_match(path, regex(".*neutralino.js$"));

        if(isClientLibrary) {
            return getAsset(path, settings::getGlobalVars());
        }
        else {
            return getAsset(path);
        }
    }

} // namespace router
