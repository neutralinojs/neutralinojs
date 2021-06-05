#include <iostream>
#include <fstream>

#if defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#elif defined(_WIN32)
#include <windows.h>
#endif 

#include "lib/json.hpp"
#include "settings.h"

using namespace std;
using json = nlohmann::json;

namespace fs {

    string createDirectory(json input) {
        json output;
        string path = input["path"];
        #if defined(__linux__) || defined(__APPLE__)
        if(mkdir(path.c_str(), 0700) == 0) {
        #elif defined(_WIN32)
        if(CreateDirectory(path.c_str(), NULL)) {
        #endif
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
        #if defined(__linux__) || defined(__APPLE__)
        if(rmdir(path.c_str()) == 0) {
        #elif defined(_WIN32)
        if(RemoveDirectory(path.c_str())) {
        #endif
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
        #if defined(__linux__) || defined(__APPLE__)
        if(remove(filename.c_str()) == 0) {
        #elif defined(_WIN32)
        if(DeleteFile(filename.c_str())) {
        #endif
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
        
        #if defined(__linux__) || defined(__APPLE__)
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
        #elif defined(_WIN32)
        string search_path = path + "/*.*";
        WIN32_FIND_DATA fd;
        HANDLE hFind = FindFirstFile(search_path.c_str(), &fd);
        if(hFind != INVALID_HANDLE_VALUE) {
            do {
                string type = "OTHER";
                if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
                    type = "DIRECTORY";
                else
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
        #endif
        return output.dump();
    }
}
