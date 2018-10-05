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
#include "../lib/json/json.hpp"
#include "auth/authbasic.h"

using namespace std;
using json = nlohmann::json;
json options;

namespace settings {

    string getFileContent(string filename){
        ifstream f;
        f.open(filename);
        stringstream strStream;
        strStream << f.rdbuf();
        return strStream.str();
    }

    json getOptions(){
        return options;
    }

    json getSettings() {
        json settings;
        try {
            settings = json::parse(getFileContent("app\\settings.json"));
        }
        catch(exception e){
            cout << e.what() << endl;
        }
        options = settings;
        return options;
    }

    string getGlobalVars(){
        json settings = getOptions();
        string s = "let NL_OS='Windows';";
        s += "let NL_NAME='" + settings["appname"].get<std::string>() + "';"; 
        s += "let NL_PORT=" + settings["appport"].get<std::string>() + ";";
        s += "let NL_MODE='" + settings["mode"].get<std::string>() + "';";
        s += "let NL_TOKEN='" + authbasic::getToken() + "';";  

        if(settings["globals"] != NULL) {
            for ( auto it: settings["globals"].items()) {
                s += "let NL_" + it.key() +  "='" + it.value().get<std::string>() + "';";
            }
        }
        return s;
    }

}