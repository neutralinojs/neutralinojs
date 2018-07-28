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
#include "../../../lib/json/json.hpp"
#include "../../settings.h"

using namespace std;
using json = nlohmann::json;

namespace storage {
    string getData(string jso) {
        json input;
        json output;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }
        string filename = "storage/" + input["bucket"].get<std::string>() + ".json";
        output["content"] = settings::getFileContent(filename);
        return output.dump();
    }

    string putData(string jso) {
        json input;
        json output;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }
        
        string filename = "storage/" + input["bucket"].get<std::string>() + ".json";
        string content = input["content"].dump();
        ofstream t(filename);
        t << content;
        t.close();
        
        return output.dump();
    } 

}