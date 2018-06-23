#include <iostream>
#include <fstream>
#include "../../lib/json/json.hpp"
#include <windows.h>
#include "../settings.h"

using namespace std;
using json = nlohmann::json;

namespace filesystem {

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
            return output.dump();
        }
        else{
            output["error"] = "Cannot create " + filename;
            return output.dump();
        }
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
            return output.dump();
        }
        else{
            output["error"] = "Cannot remove " + dir;
            return output.dump();
        }   
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
        output["content"] = settings::getFileContent(filename);
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
            return output.dump();
        }
        else{
            output["error"] = "Cannot remove " + filename;
            return output.dump();
        }   
    }
}
