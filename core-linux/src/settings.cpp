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
#include <fstream>
#include <unistd.h>
#include <limits.h>
#include <algorithm>
#include "../lib/json/json.hpp"
#include "auth/authbasic.h"
#include "resources.h"
#include "log.h"

using namespace std;
using json = nlohmann::json;
json options;
json globalArgs;
bool loadResFromDir = false;

namespace settings {

    string getFileContent(string filename) {
        if(!loadResFromDir)
            return resources::getFileContent(filename);
        ifstream t;
        t.open(filename);
        if(!t.is_open())
            return "";
        string buffer = "";
        string line;
        while(!t.eof()){
            getline(t, line);
            buffer += line + "\n";
        }
        t.close();
        return buffer;
    }

    string getFileContentBinary(string filename) {
        if(!loadResFromDir)
            return resources::getFileContent(filename);
        vector<char> buffer;
        ifstream ifd(filename.c_str(), ios::binary | ios::ate);
        if(!ifd.is_open())
            return "";
        int size = ifd.tellg();
        ifd.seekg(0, ios::beg);
        buffer.resize(size);
        ifd.read(buffer.data(), size);
        string result(buffer.begin(), buffer.end());
        return result;
    }

    string getCurrentDir() {
        char cwd[PATH_MAX];
        return getcwd(cwd, sizeof(cwd));
    }

    json getOptions(){
        return options;
    }

    void setOption(string key, string value) {
        options[key] = value;
    }

    json getSettings() {
        if(!options.is_null())
            return settings::getOptions();
        json settings;
        try {
            settings = json::parse(getFileContent("app/settings.json"));
        }
        catch(exception e){
            ERROR() << e.what();
        }
        options = settings;
        return options;
    }

    string getGlobalVars(){
        json settings = getOptions();
        string s = "var NL_OS='Linux';";
        s += "var NL_VERSION='1.8.0';";
        s += "var NL_NAME='" + settings["appname"].get<std::string>() + "';";
        s += "var NL_PORT=" + settings["appport"].get<std::string>() + ";";
        s += "var NL_MODE='" + settings["mode"].get<std::string>() + "';";
        s += "var NL_TOKEN='" + authbasic::getToken() + "';";
        s += "var NL_CWD='" + settings::getCurrentDir() + "';";
        s += "var NL_ARGS=" + globalArgs.dump() + ";";

        if(settings["globals"] != NULL) {
            for ( auto it: settings["globals"].items()) {
                s += "var NL_" + it.key() +  "='" + it.value().get<std::string>() + "';";
            }
        }
        return s;
    }

    void setGlobalArgs(json args) {
        globalArgs = args;
        loadResFromDir = std::find(globalArgs.begin(), globalArgs.end(), "--load-dir-res") != globalArgs.end();
    }

}
