// MIT License

// Copyright (c) 2018 Neutralinojs

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iostream>
#include <string>
#include <fstream>
#include <unistd.h>
#include <regex>
#include <vector>
#include "helpers.h"
#include "settings.h"
#include "lib/json.hpp"
#include "auth/authbasic.h"
#include "cloud/privileges.h"

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
typedef string (*pfunc)(string);

namespace routes {

    pair<string, string> executeNativeMethod(string path, string postData, string token) {
        string modfunc = regex_replace(path, std::regex("/__nativeMethod_"), "");
        string output = "";

        if(!authbasic::verifyToken(token))
            return make_pair("{\"error\":\"Invalid or expired NL_TOKEN value from client.\"}", "application/json");
        if(settings::getMode() == "cloud") {
            if(!privileges::checkPermission(modfunc))
                return make_pair("{\"error\":\"Missing permission! Blocked action in cloud mode\"}", "application/json");
        }
        // TODO: Try to make the following code dynamic
        if(fs::funcmap.find(modfunc) != fs::funcmap.end() ){
            pfunc f = fs::funcmap[modfunc];
            output = (*f)(postData);
        }
        else if(os::funcmap.find(modfunc) != os::funcmap.end() ){
            pfunc f = os::funcmap[modfunc];
            output = (*f)(postData);
        }
        else if(computer::funcmap.find(modfunc) != computer::funcmap.end() ){
            pfunc f = computer::funcmap[modfunc];
            output = (*f)(postData);
        }
        else if(storage::funcmap.find(modfunc) != storage::funcmap.end() ){
            pfunc f = storage::funcmap[modfunc];
            output = (*f)(postData);
        }
        else if(debug::funcmap.find(modfunc) != debug::funcmap.end() ){
            pfunc f = debug::funcmap[modfunc];
            output = (*f)(postData);
        }
        else if(app::funcmap.find(modfunc) != app::funcmap.end() ){
            pfunc f = app::funcmap[modfunc];
            output = (*f)(postData);
        }
        else {
            json defaultOutput = {{"error", modfunc + " is not implement in the Neutralinojs server."}};
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
        string fileData = settings::getFileContentBinary(path);
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
