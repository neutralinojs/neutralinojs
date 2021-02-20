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
#include <windows.h>

using namespace std;
using json = nlohmann::json;

namespace fs {

    string createDirectory(string jso) {
        json input;
        json output;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }
        string filename = input["dir"];
        if(CreateDirectory(filename.c_str(), NULL)){
            output["success"] = true;
        }
        else{
            output["error"] = "Cannot create " + filename;
        }
        return output.dump();
    }

    string removeDirectory(string jso) {
        json input;
        json output;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }
        string dir = input["dir"];
        if(RemoveDirectory(dir.c_str())){
            output["success"] = true;
        }
        else{
            output["error"] = "Cannot remove " + dir;
        }   
        return output.dump();
    }

    string readFile(string jso) {
        json input;
        json output;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }
        string filename = input["filename"];
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
        output["content"] = buffer;
        return output.dump();
    }

     string writeFile(string jso) {
        json input;
        json output;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }
        string filename = input["filename"];
        string content = input["content"];
        ofstream t(filename);
        t << content;
        t.close();
        output["success"] = true;
        return output.dump();
    }   

    string removeFile(string jso) {
        json input;
        json output;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }
        string filename = input["filename"];
        if(DeleteFile(filename.c_str())){
            output["success"] = true;
        }
        else{
            output["error"] = "Cannot remove " + filename;
        }   
        return output.dump();
    }

    string readDirectory(string jso) {
        json input;
        json output;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }
        string path = input["path"];
        string search_path = path + "/*.*";
        WIN32_FIND_DATA fd; 
        HANDLE hFind = FindFirstFile(search_path.c_str(), &fd); 
        if(hFind != INVALID_HANDLE_VALUE) { 
            do { 
                string type = "other";
                if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
                    type = "directory";
                else if((fd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) == FILE_ATTRIBUTE_ARCHIVE)
                    type = "file";
                
                json file = {
                    {"name", fd.cFileName},
                    {"type", type}
                };
                output["files"].push_back(file);
            } while(FindNextFile(hFind, &fd)); 
            FindClose(hFind); 
        }
        return output.dump();
    }
}
