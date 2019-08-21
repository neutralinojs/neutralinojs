#include <iostream>
#include <fstream>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../../../lib/json/json.hpp"
#include "../../settings.h"

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
        string filename = input["name"];
        if(mkdir(filename.c_str(), 0700) == 0){
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
        if(rmdir(dir.c_str()) == 0){
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
        if(remove(filename.c_str()) == 0){
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
            
        DIR *dirp;
        struct dirent *directory;

        dirp = opendir(path.c_str());
        if (dirp) {
            while ((directory = readdir(dirp)) != NULL) {
                string type = "other";
                if(directory->d_type == DT_DIR)
                    type = "directory";
                else if(directory->d_type == DT_REG)
                    type = "file";
                json file = {
                    {"name", directory->d_name},
                    {"type", type},
                };
                output["files"].push_back(file);
            }
            closedir(dirp);
        }

        return output.dump();
    }
}
