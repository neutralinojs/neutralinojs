#include <iostream>
#include <fstream>
#include "lib/json.hpp"
#include <windows.h>

using namespace std;
using json = nlohmann::json;

namespace fs {

    string createDirectory(json input) {
        json output;
        string path = input["path"];
        if(CreateDirectory(path.c_str(), NULL)) {
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
        if(RemoveDirectory(path.c_str())){
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
            output["error"] = "Unable to open file " + filename;
            return output.dump();
        }
        string buffer = "";
        string line;
        while(!t.eof()){
            getline(t, line);
            buffer += line + "\n";
        }
        t.close();
        output["data"] = buffer;
        output["success"] = true;
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
        if(DeleteFile(filename.c_str())){
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
        string search_path = path + "/*.*";
        WIN32_FIND_DATA fd;
        HANDLE hFind = FindFirstFile(search_path.c_str(), &fd);
        if(hFind != INVALID_HANDLE_VALUE) {
            do {
                string type = "OTHER";
                if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
                    type = "DIRECTORY";
                else if((fd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) == FILE_ATTRIBUTE_ARCHIVE)
                    type = "FILE";

                json file = {
                    {"entry", fd.cFileName},
                    {"type", type}
                };
                output["entries"].push_back(file);
            } while(FindNextFile(hFind, &fd));
            FindClose(hFind);
            output["success"] = true;
        }
        return output.dump();
    }
}
