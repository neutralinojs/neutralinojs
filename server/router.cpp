#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <vector>
#include <filesystem>

#include <websocketpp/server.hpp>

#include "lib/json/json.hpp"
#include "auth/authbasic.h"
#include "auth/permission.h"
#include "server/router.h"
#include "server/neuserver.h"
#include "helpers.h"
#include "errors.h"
#include "settings.h"
#include "resources.h"
#include "api/os/os.h"
#include "api/fs/fs.h"
#include "api/computer/computer.h"
#include "api/storage/storage.h"
#include "api/debug/debug.h"
#include "api/app/app.h"
#include "api/window/window.h"
#include "api/events/events.h"
#include "api/extensions/extensions.h"
#include "api/clipboard/clipboard.h"
#include "api/res/res.h"
#include "api/server/server.h"
#include "api/custom/custom.h"

#if defined(__APPLE__)
#include <dispatch/dispatch.h>
#endif

using namespace std;

using json = nlohmann::json;

namespace router {

map<string, router::NativeMethod> methodMap = {
    // Neutralino.app
    {"app.exit", app::controllers::exit},
    {"app.killProcess", app::controllers::killProcess},
    {"app.getConfig", app::controllers::getConfig},
    {"app.broadcast", app::controllers::broadcast},
    {"app.readProcessInput", app::controllers::readProcessInput},
    {"app.writeProcessOutput", app::controllers::writeProcessOutput},
    {"app.writeProcessError", app::controllers::writeProcessError},
    // Neutralino.window
    {"window.setTitle", window::controllers::setTitle},
    {"window.getTitle", window::controllers::getTitle},
    {"window.maximize", window::controllers::maximize},
    {"window.isMaximized", window::controllers::isMaximized},
    {"window.unmaximize", window::controllers::unmaximize},
    {"window.minimize", window::controllers::minimize},
    {"window.unminimize", window::controllers::unminimize},
    {"window.isMinimized", window::controllers::isMinimized},
    {"window.isVisible", window::controllers::isVisible},
    {"window.show", window::controllers::show},
    {"window.hide", window::controllers::hide},
    {"window.isFullScreen", window::controllers::isFullScreen},
    {"window.setFullScreen", window::controllers::setFullScreen},
    {"window.exitFullScreen", window::controllers::exitFullScreen},
    {"window.focus", window::controllers::focus},
    {"window.setIcon", window::controllers::setIcon},
    {"window.move", window::controllers::move},
    {"window.beginDrag", window::controllers::beginDrag},
    {"window.center", window::controllers::center},
    {"window.setSize", window::controllers::setSize},
    {"window.getSize", window::controllers::getSize},
    {"window.getPosition", window::controllers::getPosition},
    {"window.setAlwaysOnTop", window::controllers::setAlwaysOnTop},
    {"window.snapshot", window::controllers::snapshot},
    {"window.setMainMenu", window::controllers::setMainMenu},
    {"window.print", window::controllers::print},
    // Neutralino.computer
    {"computer.getMemoryInfo", computer::controllers::getMemoryInfo},
    {"computer.getArch", computer::controllers::getArch},
    {"computer.getKernelInfo", computer::controllers::getKernelInfo},
    {"computer.getOSInfo", computer::controllers::getOSInfo},
    {"computer.getCPUInfo", computer::controllers::getCPUInfo},
    {"computer.getDisplays", computer::controllers::getDisplays},
    {"computer.getMousePosition", computer::controllers::getMousePosition},
    // Neutralino.debug
    {"debug.log", debug::controllers::log},
    // Neutralino.filesystem
    {"filesystem.createDirectory", fs::controllers::createDirectory},
    {"filesystem.remove", fs::controllers::remove},
    {"filesystem.readFile", fs::controllers::readFile},
    {"filesystem.readBinaryFile", fs::controllers::readBinaryFile},
    {"filesystem.writeFile", fs::controllers::writeFile},
    {"filesystem.writeBinaryFile", fs::controllers::writeBinaryFile},
    {"filesystem.appendFile", fs::controllers::appendFile},
    {"filesystem.appendBinaryFile", fs::controllers::appendBinaryFile},
    {"filesystem.openFile", fs::controllers::openFile},
    {"filesystem.createWatcher", fs::controllers::createWatcher},
    {"filesystem.removeWatcher", fs::controllers::removeWatcher},
    {"filesystem.getWatchers", fs::controllers::getWatchers},
    {"filesystem.updateOpenedFile", fs::controllers::updateOpenedFile},
    {"filesystem.getOpenedFileInfo", fs::controllers::getOpenedFileInfo},
    {"filesystem.readDirectory", fs::controllers::readDirectory},
    {"filesystem.copy", fs::controllers::copy},
    {"filesystem.move", fs::controllers::move},
    {"filesystem.getStats", fs::controllers::getStats},
    {"filesystem.getAbsolutePath", fs::controllers::getAbsolutePath},
    {"filesystem.getRelativePath", fs::controllers::getRelativePath},
    {"filesystem.getPathParts", fs::controllers::getPathParts},
    {"filesystem.getPermissions", fs::controllers::getPermissions},
    {"filesystem.setPermissions", fs::controllers::setPermissions},
    {"filesystem.getJoinedPath", fs::controllers::getJoinedPath},
    {"filesystem.getNormalizedPath", fs::controllers::getNormalizedPath},
    {"filesystem.getUnnormalizedPath", fs::controllers::getUnnormalizedPath},
    // Neutralino.os
    {"os.execCommand", os::controllers::execCommand},
    {"os.spawnProcess", os::controllers::spawnProcess},
    {"os.updateSpawnedProcess", os::controllers::updateSpawnedProcess},
    {"os.getSpawnedProcesses", os::controllers::getSpawnedProcesses},
    {"os.getEnv", os::controllers::getEnv},
    {"os.getEnvs", os::controllers::getEnvs},
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
    {"storage.getData", storage::controllers::getData},
    {"storage.removeData", storage::controllers::removeData},
    {"storage.getKeys", storage::controllers::getKeys},
    {"storage.clear", storage::controllers::clear},
    // Neutralino.events
    {"events.broadcast", events::controllers::broadcast},
    // Neutralino.extensions
    {"extensions.dispatch", extensions::controllers::dispatch},
    {"extensions.broadcast", extensions::controllers::broadcast},
    {"extensions.getStats", extensions::controllers::getStats},
    // Neutralino.clipboard
    {"clipboard.getFormat", clipboard::controllers::getFormat},
    {"clipboard.readText", clipboard::controllers::readText},
    {"clipboard.readImage", clipboard::controllers::readImage},
    {"clipboard.writeText", clipboard::controllers::writeText},
    {"clipboard.writeHTML", clipboard::controllers::writeHTML},
    {"clipboard.readHTML", clipboard::controllers::readHTML},
    {"clipboard.writeImage", clipboard::controllers::writeImage},
    {"clipboard.clear", clipboard::controllers::clear},
    // Neutralino.resources
    {"resources.getFiles", res::controllers::getFiles},
    {"resources.getStats", res::controllers::getStats},
    {"resources.extractFile", res::controllers::extractFile},
    {"resources.extractDirectory", res::controllers::extractDirectory},
    {"resources.readFile", res::controllers::readFile},
    {"resources.readBinaryFile", res::controllers::readBinaryFile},
    // Neutralino.server
    {"server.mount", server::controllers::mount},
    {"server.unmount", server::controllers::unmount},
    {"server.getMounts", server::controllers::getMounts},
    // Neutralino.custom
    {"custom.getMethods", custom::controllers::getMethods},
    // {"custom.add", custom::controllers::add} // Sample custom method
};

map<string, router::NativeMethod> getMethodMap() {
    return methodMap;
}

router::NativeMessage executeNativeMethod(const router::NativeMessage &request) {
    string nativeMethodId = request.method;
    router::NativeMessage response;
    response.id = request.id;
    response.method = request.method;

    if(!authbasic::verifyToken(request.accessToken)) {
        response.data["error"] = errors::makeErrorPayload(errors::NE_RT_INVTOKN);
        return response;
    }
    if(!permission::hasAPIAccess()) {
        response.data["error"] = errors::makeErrorPayload(errors::NE_RT_APIPRME);
        return response;
    }
    if(!permission::hasMethodAccess(nativeMethodId)) {
        response.data["error"] = errors::makeErrorPayload(errors::NE_RT_NATPRME, nativeMethodId);
        return response;
    }

    if(methodMap.find(nativeMethodId) != methodMap.end()) {
        try {
            if(settings::getMode() != settings::AppModeWindow
                && regex_match(nativeMethodId, regex("^window.*"))) {
                response.data["success"] = true;
                response.data["message"] = "Discarded. "+ nativeMethodId + " works within the window mode only";
                return response;
            }
            router::NativeMethod nativeMethod = methodMap[nativeMethodId];
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
            response.data["error"] = errors::makeErrorPayload(errors::NE_RT_NATRTER);
            return response;
        }
    }
    else {
        response.data["error"] = errors::makeErrorPayload(errors::NE_RT_NATNTIM, nativeMethodId);
        return response;
    }
}

map<string, string> mountedPaths = {};

errors::StatusCode mountPath(string &path, string &target) {
    path = helpers::normalizePath(path);
    target = helpers::normalizePath(target);

    if(path.empty()) {
        path = "/";
    }

    const auto targetPath = filesystem::path(CONVSTR(target));
    
    if(!filesystem::exists(targetPath)) {
        return errors::NE_FS_NOPATHE;
    }
    if(!filesystem::is_directory(targetPath)) {
        return errors::NE_FS_NOTADIR;
    }
    if(router::isMounted(path)) {
        return errors::NE_SR_MPINUSE;
    }

    mountedPaths[path] = target;
    return errors::NE_ST_OK;
}

bool isMounted(const string &path) {
    return mountedPaths.find(path) != mountedPaths.end();
}

bool unmountPath(string &path) {
    path = helpers::normalizePath(path);
    
    if(path.empty()) {
        path = "/";
    }
    if(!router::isMounted(path)) {
        return false;
    }
    
    mountedPaths.erase(path);
    return true;
}

map<string, string> getMounts() {
    return mountedPaths;
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

    fs::FileReaderResult fileReaderResult;
    bool foundMountedPath = false;

    if(mountedPaths.size() > 0) {
        string pathname = path;
        string documentRoot = neuserver::getDocumentRoot();
        if(!documentRoot.empty()) {
            pathname = path.substr(documentRoot.length());
        }
        for(const auto& [mountedPath, mountTarget] : mountedPaths) {
            if(pathname.find(mountedPath) == 0) {
                string adjustedPath = mountTarget + "/" + pathname.substr(mountedPath.length());
                fileReaderResult = fs::readFile(adjustedPath);
                foundMountedPath = true;
                break;
            }
        }
    }

    if(!foundMountedPath) {
        fileReaderResult = resources::getFile(path);
    }
    
    if(fileReaderResult.status != errors::NE_ST_OK) {
        json jSpaServing = settings::getOptionForCurrentMode("singlePageServe");
        if(!jSpaServing.is_null() && jSpaServing.get<bool>() && regex_match(path, regex(".*index.html$"))) {
            json jDocumentRoot = settings::getOptionForCurrentMode("documentRoot");
            string newPath;
            if(!jDocumentRoot.is_null()) {
                newPath = jDocumentRoot.get<string>() + "index.html";
            }
            else {
                newPath = settings::getNavigationUrl();
            }
            if(newPath != path) return getAsset(newPath, prependData);
        }
    }
    response.data = fileReaderResult.data;
    response.status = websocketpp::http::status_code::ok;

    if(fileReaderResult.status != errors::NE_ST_OK) {
        response.status = websocketpp::http::status_code::not_found;
        debug::log(debug::LogTypeError, errors::makeErrorMsg(errors::NE_RS_UNBLDRE, path));
    }
    else if(prependData != "") {
        response.data = prependData + response.data;
    }

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

    // Ignore query params
    path = path.substr(0, path.find("?"));

    bool isClientLibrary = regex_match(path, regex(".*neutralino.js$"));
    bool isGlobalsRequest = regex_match(path, regex(".*__neutralino_globals.js$"));

    if(isClientLibrary) {
        return getAsset(path, settings::getGlobalVars());
    }
    else if(isGlobalsRequest) {
        return router::Response {
            websocketpp::http::status_code::ok,
            "application/javascript",
            settings::getGlobalVars()
        };
    }
    else {
        return getAsset(path);
    }
}

} // namespace router
