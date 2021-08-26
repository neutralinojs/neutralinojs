#include <iostream>
#include <fstream>

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>

#elif defined(_WIN32)
#include <windows.h>
#include <atlstr.h>
#include <shlwapi.h>
#include <winbase.h>
#endif 

#include "lib/json.hpp"
#include "lib/base64.hpp"
#include "settings.h"
#include "api/filesystem/filesystem.h"
#include "api/os/os.h"

using namespace std;
using json = nlohmann::json;

namespace fs {
    bool createDirectory(string path) {
        #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
        return mkdir(path.c_str(), 0700) == 0;
        #elif defined(_WIN32)
        return CreateDirectory(path.c_str(), nullptr) == 1;
        #endif
    }
    
    bool removeFile(string filename) {
        #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
        return remove(filename.c_str()) == 0;
        #elif defined(_WIN32)
        return DeleteFile(filename.c_str()) == 1;
        #endif
    }
    
    string getDirectoryName(string filename){
        #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
        return dirname((char*)filename.c_str());
        #elif defined(_WIN32)
        LPSTR pathToReplace = const_cast<char *>(filename.c_str());
        PathRemoveFileSpecA(pathToReplace);
        std::string directory(pathToReplace);
        std::replace(directory.begin(), directory.end(), '\\', '/');
        return directory;
        #endif 
    }

    string getCurrentDirectory() {
        #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
        return getcwd(nullptr, 0);
        #elif defined(_WIN32)
        TCHAR currentDir[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, currentDir);   
        std::string currentDirStr(currentDir);
        std::replace(currentDirStr.begin(), currentDirStr.end(), '\\', '/');     
        return currentDirStr;
        #endif
    }
    
    string getFullPathFromRelative(string path) {
        #if defined(__linux__)
        return realpath(path.c_str(), nullptr);
        #else
        return path;
        #endif
    }
    
    fs::FileReaderResult readFile(string filename) {
        fs::FileReaderResult fileReaderResult;
        ifstream reader(filename.c_str(), ios::binary | ios::ate);
        if(!reader.is_open()) {
            fileReaderResult.hasError = true;
            fileReaderResult.error = "Unable to open " + filename;
            return fileReaderResult;
        }
        vector<char> buffer;
        int size = reader.tellg();
        reader.seekg(0, ios::beg);
        buffer.resize(size);
        reader.read(buffer.data(), size);
        string result(buffer.begin(), buffer.end());
        reader.close();
        
        fileReaderResult.data = result;
        return fileReaderResult;
    }

     bool writeFile(fs::FileWriterOptions fileWriterOptions) {
        json output;
        ofstream writer(fileWriterOptions.filename);
        if(!writer.is_open())
            return false;
        writer << fileWriterOptions.data;
        writer.close();
        return true;
    }
    
    fs::FileStats getStats(string path) {
        fs::FileStats fileStats;
        #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
        struct stat statBuffer;
        if(stat(path.c_str(), &statBuffer) == 0) {
            fileStats.size = statBuffer.st_size;
            fileStats.isFile = S_ISREG(statBuffer.st_mode);
            fileStats.isDirectory = S_ISDIR(statBuffer.st_mode);
        }
        else {
            fileStats.hasError = true;
            fileStats.error = path + " doesn't exists or access error";
        }
        #elif defined(_WIN32)
 
        #endif
        return fileStats;
    }

namespace controllers {
    json createDirectory(json input) {
        json output;
        string path = input["path"];
        if(fs::createDirectory(path)) {
            output["success"] = true;
            output["message"] = "Directory " + path + " was created";
        }
        else{
            output["error"] = "Cannot create a directory in " + path;
        }
        return output;
    }

    json removeDirectory(json input) {
        json output;
        string path = input["path"];
        #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
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
        return output;
    }

    json readFile(json input) {
        json output;
        fs::FileReaderResult fileReaderResult;
        fileReaderResult = fs::readFile(input["fileName"].get<std::string>());
        if(fileReaderResult.hasError) {
            output["error"] = fileReaderResult.error;
        }
        else {
            output["data"] = fileReaderResult.data;
            output["success"] = true;
        }
        return output;
    }
    
    json readBinaryFile(json input) {
        json output;
        fs::FileReaderResult fileReaderResult;
        fileReaderResult = fs::readFile(input["fileName"].get<std::string>());
        if(fileReaderResult.hasError) {
            output["error"] = fileReaderResult.error;
        }
        else {
            output["returnValue"] = base64::to_base64(fileReaderResult.data);
            output["success"] = true;
        }
        return output;
    }

    json writeFile(json input) {
        json output;
        fs::FileWriterOptions fileWriterOptions;
        fileWriterOptions.filename = input["fileName"];
        fileWriterOptions.data = input["data"];
        if(fs::writeFile(fileWriterOptions))
            output["success"] = true;
        else
            output["error"] = "Unable to write file: " + fileWriterOptions.filename;
        return output;
    }
    
    json writeBinaryFile(json input) {
        json output;
        fs::FileWriterOptions fileWriterOptions;
        fileWriterOptions.filename = input["fileName"];
        fileWriterOptions.data = base64::from_base64(input["data"].get<std::string>());
        if(fs::writeFile(fileWriterOptions))
            output["success"] = true;
        else
            output["error"] = "Unable to write file: " + fileWriterOptions.filename;
        return output;
    }

    json removeFile(json input) {
        json output;
        string filename = input["fileName"];
        if(fs::removeFile(filename)) {
            output["success"] = true;
            output["message"] = filename + " was deleted";
        }
        else{
            output["error"] = "Cannot remove " + filename;
        }
        return output;
    }

    json readDirectory(json input) {
        json output;
        string path = input["path"];
        
        #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
        DIR *dirp;
        struct dirent *directory;
        dirp = opendir(path.c_str());
        if (dirp) {
            while ((directory = readdir(dirp)) != nullptr) {
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
        return output;
    }
   
    json copyFile(json input) {
        json output;
        string source = input["source"];
        string destination = input["destination"];
        #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
        string command = "cp \"" + source + "\" \"" + destination + "\"";
        string commandOutput = os::execCommand(command, true);
        if(commandOutput.empty()) {

        #elif defined(_WIN32)
        if(CopyFile(source.c_str(), destination.c_str(), false) == 1) {
        #endif
            output["success"] = true;
            output["message"] = "File copy operation was successful";
        }
        else{
            output["error"] = "Cannot copy " + source + " to " + destination;
        }
        return output;
    } 
    
    json moveFile(json input) {
        json output;
        string source = input["source"];
        string destination = input["destination"];
        #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
        string command = "mv \"" + source + "\" \"" + destination + "\"";
        string commandOutput = os::execCommand(command, true);
        if(commandOutput.empty()) {

        #elif defined(_WIN32)
        if(MoveFile(source.c_str(), destination.c_str()) == 1) {
        #endif
            output["success"] = true;
            output["message"] = "File move operation was successful";
        }
        else{
            output["error"] = "Cannot move " + source + " to " + destination;
        }
        return output;
    } 
    
    json getStats(json input) {
        json output;
        string path = input["path"];
        fs::FileStats fileStats = fs::getStats(path);
        if(!fileStats.hasError) {
            output["success"] = true;
            output["size"] = fileStats.size;
            output["isFile"] = fileStats.isFile;
            output["isDirectory"] = fileStats.isDirectory;
        }
        else{
            output["error"] = fileStats.error;
        }
        return output;
    } 
    
} // namespace controllers
} // namespace fs
