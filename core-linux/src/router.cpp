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
#include "functions.h"
#include "settings.h"
#include "core/filesystem/filesystem.h"
#include "core/os/os.h"
#include "core/computer/computer.h"
#include "core/storage/storage.h"
#include "core/debug/debug.h"
#include "core/app/app.h"
#include "../lib/json/json.hpp"
#include "auth/authbasic.h"
#include "ping/ping.h"
#include "cloud/privileges.h"

using namespace std;
using namespace filesystem;
using json = nlohmann::json;

namespace routes {

    string getClientJs() {
        return settings::getFileContent("app/assets/neutralino.js");
    }

    string getIcon() {
        return "data:image/png;base64,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";
    }

    string getIndex() {
        return "<html style=\"width: 100%; height: 100%; position: absolute; background-repeat:no-repeat; background-position: center; background-color: black; background-image:url('" + routes::getIcon() + "')\"></html>";
    }

    pair<string, string> getAsset(string path, bool isBinary = false) {
        vector<string> split = functions::split(path, '.');
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
        if(isBinary)
            return make_pair( settings::getFileContentBinary("app" + path), mimeTypes[extension]);
        else
            return make_pair( settings::getFileContent("app" + path), mimeTypes[extension]);
    }

   pair<string, string> handle(string encodedPath, string j, string token) {
        char *originalPath = (char *) encodedPath.c_str();
        char *decodedPath = new char[strlen(originalPath) + 1];
        functions::urldecode(decodedPath, originalPath);

        string path = string(decodedPath);
        json options = settings::getOptions();
        ping::receivePing();

        string appname = options["appname"];
        bool isAsset = path.find("/assets") != string::npos;
        if(path == "/" +  appname ){
            return make_pair(settings::getFileContent("app/index.html"), "text/html");
        }
        else if(path == "/neutralino.js"){
            return make_pair(settings::getGlobalVars() + routes::getClientJs(), "text/javascript");
        }
        else if(path == "/settings.json"){
            return make_pair(settings::getSettings().dump(), "application/json");
        }
        else if(isAsset && regex_match(path, regex(".*\\.(js|html|css|json|xml)$"))) {
            return getAsset(path);
        }
        else if(isAsset && regex_match(path, regex(".*\\.(jpg|png|svg|gif|ico|woff2|mp3)$"))) {
            return getAsset(path, true);
        }
        else if(isAsset) {
            return make_pair("{\"error\":\"Unsupported file type!\"}", "application/json");;
        }
        else if(path == "/") {
            return make_pair(routes::getIndex(), "text/html");
        }
        else {
            vector<string> portions = functions::split(path, '/');
            if(portions.size() == 3) {
                if(authbasic::verifyToken(token)) {
                    string module = portions[1];
                    string func = portions[2];
                    string modfunc = module + "." + func;
                    string output = "";

                    bool permission = true;

                    if(privileges::getMode() == "cloud") {
                        if(!privileges::checkPermission(modfunc)) permission = false;
                    }

                    if(permission) {

                        if(filesystem::funcmap.find(modfunc) != filesystem::funcmap.end() ){
                            pfunc f = filesystem::funcmap[modfunc];
                            output = (*f)(j);
                        }
                        else if(os::funcmap.find(modfunc) != os::funcmap.end() ){
                            pfunc f = os::funcmap[modfunc];
                            output = (*f)(j);
                        }
                        else if(computer::funcmap.find(modfunc) != computer::funcmap.end() ){
                            pfunc f = computer::funcmap[modfunc];
                            output = (*f)(j);
                        }
                        else if(storage::funcmap.find(modfunc) != storage::funcmap.end() ){
                            pfunc f = storage::funcmap[modfunc];
                            output = (*f)(j);
                        }
                        else if(debug::funcmap.find(modfunc) != debug::funcmap.end() ){
                            pfunc f = debug::funcmap[modfunc];
                            output = (*f)(j);
                        }
                        else if(app::funcmap.find(modfunc) != app::funcmap.end() ){
                            pfunc f = app::funcmap[modfunc];
                            output = (*f)(j);
                        }
                        else {
                            json o = {{"error", modfunc + " is not supported"}};
                            output = o.dump();
                        }

                        return make_pair(output, "application/json");
                    }

                    else {
                         return make_pair("{\"error\":\"Cloud permission error!\"}", "application/json");
                    }

                }
                else {
                    return make_pair("{\"error\":\"Authentication error!\"}", "application/json");
                }


            }
        }
        delete []decodedPath;
        return make_pair("{\"message\":\"Neutralino\"}", "application/json");
    }


}
