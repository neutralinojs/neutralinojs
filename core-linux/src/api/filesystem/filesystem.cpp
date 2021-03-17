#include <iostream>
#include <fstream>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lib/json.hpp"
#include "settings.h"

using namespace std;
using json = nlohmann::json;

namespace fs {

    string createDirectory(json input) {
        json output;
        string path = input["path"];
        if(mkdir(path.c_str(), 0700) == 0){
            output["success"] = true;
            output["message"] = "Directory " + path + " was created";
        }
        else{
            output["error"] = "Cannot create a directory in " + path;
        }
        return output.dump();
    }

    string removeDirectory(json input) {
        json output;
        string path = input["path"];
        if(rmdir(path.c_str()) == 0){
            output["success"] = true;
            output["message"] = "Directory " + path + " was removed";
        }
        else{
            output["error"] = "Cannot remove " + path;
        }
        return output.dump();
    }

    string readFile(json input) {
        json output;
        string filename = input["fileName"];
        ifstream t;
        t.open(filename);
        if(!t.is_open()) {
            output["error"] = "Unable to open " + filename;
            return output.dump();
        }
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

     string writeFile(json input) {
        json output;
        string filename = input["fileName"];
        string data = input["data"];
        ofstream t(filename);
        t << data;
        t.close();
        output["success"] = true;
        return output.dump();
    }

    string removeFile(json input) {
        json output;
        string filename = input["fileName"];
        if(remove(filename.c_str()) == 0){
            output["success"] = true;
            output["message"] = filename + " was deleted";
        }
        else{
            output["error"] = "Cannot remove " + filename;
        }
        return output.dump();
    }

    string readDirectory(json input) {
        json output;
        string path = input["path"];

        DIR *dirp;
        struct dirent *directory;

        dirp = opendir(path.c_str());
        if (dirp) {
            while ((directory = readdir(dirp)) != NULL) {
                string type = "OTHER";
                if(directory->d_type == DT_DIR)
                    type = "DIRECTORY";
                else if(directory->d_type == DT_REG)
                    type = "FILE";
                json file = {
                    {"entry", directory->d_name},
                    {"type", type},
                };
                output["entries"].push_back(file);
            }
            closedir(dirp);
            output["success"] = true;
        }
        return output.dump();
    }
}
