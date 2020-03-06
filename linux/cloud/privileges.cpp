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
#include <chrono>
#include <thread>
#include <vector>
#include "../../lib/json/json.hpp"
#include "../settings.h"

using namespace std;
using json = nlohmann::json;





namespace privileges {

    string mode = "";
    vector <string> blacklist;

    
    string getMode() {
        if(mode != "") {
            return mode;
        }
        else {
            json options = settings::getOptions()["mode"];
            return options;
        }

    }
    
    vector<string> getBlacklist() {
        if(blacklist.size() != 0 || privileges::getMode() == "browser") {
            return blacklist;
        }
        else if(privileges::getMode() == "cloud") {
            json options = settings::getOptions()["cloud"]["blacklist"];
            vector<string> s = options;
            blacklist = s;
            return blacklist;
        }
        return vector<string>();
    }


    bool checkPermission(string func) {
        for(int i = 0; i < blacklist.size(); i++) {
            if(blacklist[i] == func) return false;
        }
        return true;
    }



}