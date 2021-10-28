#include <iostream>
#include <fstream>

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>

#elif defined(_WIN32)
#define _WINSOCKAPI_
#include <windows.h>
#include <atlstr.h>
#include <shlwapi.h>
#include <winbase.h>
#endif 

#include "lib/json/json.hpp"
#include "lib/base64/base64.hpp"
#include "settings.h"
#include "helpers.h"
#include "api/filesystem/filesystem.h"
#include "api/os/os.h"

using namespace std;
using json = nlohmann::json;

namespace fs {
    bool createDirectory(const string &path) {
        #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
        return mkdir(path.c_str(), 0700) == 0;
        #elif defined(_WIN32)
        return CreateDirectory(path.c_str(), nullptr) == 1;
        #endif
    }
    
    bool removeFile(const string &filename) {
        #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
        return remove(filename.c_str()) == 0;
        #elif defined(_WIN32)
        return DeleteFile(filename.c_str()) == 1;
        #endif
    }
    
    string getDirectoryName(const string &filename){
        #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
        return dirname((char*)filename.c_str());
        #elif defined(_WIN32)
        LPSTR pathToReplace = const_cast<char *>(filename.c_str());
        PathRemoveFileSpecA(pathToReplace);
        string directory(pathToReplace);
        replace(directory.begin(), directory.end(), '\\', '/');
        return directory;
        #endif 
    }

    string getCurrentDirectory() {
        #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
        return getcwd(nullptr, 0);
        #elif defined(_WIN32)
        TCHAR currentDir[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, currentDir);   
        string currentDirStr(currentDir);
        replace(currentDirStr.begin(), currentDirStr.end(), '\\', '/');     
        return currentDirStr;
        #endif
    }
    
    string getFullPathFromRelative(const string &path) {
        #if defined(__linux__)
        return realpath(path.c_str(), nullptr);
        #else
        return path;
        #endif
    }
    
    fs::FileReaderResult readFile(const string &filename) {
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

     bool writeFile(const fs::FileWriterOptions &fileWriterOptions) {
        json output;
        ofstream writer(fileWriterOptions.filename);
        if(!writer.is_open())
            return false;
        writer << fileWriterOptions.data;
        writer.close();
        return true;
    }
    
    fs::FileStats getStats(const string &path) {
        fs::FileStats fileStats;
        #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
        struct stat statBuffer;
        if(stat(path.c_str(), &statBuffer) == 0) {
            fileStats.size = statBuffer.st_size;
            fileStats.isFile = S_ISREG(statBuffer.st_mode);
            fileStats.isDirectory = S_ISDIR(statBuffer.st_mode);
        }

        #elif defined(_WIN32)
        HANDLE hFile = CreateFile(path.c_str(), GENERIC_READ, 
                        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);

        LARGE_INTEGER size;
        FILE_BASIC_INFO basicInfo;
        if(GetFileSizeEx(hFile, &size) && 
            GetFileInformationByHandleEx(hFile, FileBasicInfo, &basicInfo, sizeof(basicInfo))
        ) {
            fileStats.size = size.QuadPart;
            fileStats.isFile = !(basicInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY);
            fileStats.isDirectory = basicInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        }

        #endif
        else {
            fileStats.hasError = true;
            fileStats.error = path + " doesn't exists or access error";
        }
        #if defined(_WIN32)
        CloseHandle(hFile);
        #endif
        return fileStats;
    }

namespace controllers {
    json createDirectory(const json &input) {
        json output;
        if(!input.contains("path")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        string path = input["path"].get<string>();
        if(fs::createDirectory(path)) {
            output["success"] = true;
            output["message"] = "Directory " + path + " was created";
        }
        else{
            output["error"] = helpers::makeErrorPayload("NE_FS_DIRCRER", 
                                "Cannot create a directory in " + path);
        }
        return output;
    }

    json removeDirectory(const json &input) {
        json output;
        if(!input.contains("path")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        string path = input["path"].get<string>();
        #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
        if(rmdir(path.c_str()) == 0) {
        #elif defined(_WIN32)
        if(RemoveDirectory(path.c_str())) {
        #endif
            output["success"] = true;
            output["message"] = "Directory " + path + " was removed";
        }
        else{
            output["error"] = helpers::makeErrorPayload("NE_FS_RMDIRER", 
                                "Cannot remove " + path);
        }
        return output;
    }

    json readFile(const json &input) {
        json output;
        if(!input.contains("path")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        fs::FileReaderResult fileReaderResult;
        fileReaderResult = fs::readFile(input["path"].get<string>());
        if(fileReaderResult.hasError) {
            output["error"] = helpers::makeErrorPayload("NE_FS_FILRDER", fileReaderResult.error);
        }
        else {
            output["returnValue"] = fileReaderResult.data;
            output["success"] = true;
        }
        return output;
    }
    
    json readBinaryFile(const json &input) {
        json output;
        if(!input.contains("path")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        fs::FileReaderResult fileReaderResult;
        fileReaderResult = fs::readFile(input["path"].get<string>());
        if(fileReaderResult.hasError) {
            output["error"] = helpers::makeErrorPayload("NE_FS_FILRDER", fileReaderResult.error);
        }
        else {
            output["returnValue"] = base64::to_base64(fileReaderResult.data);
            output["success"] = true;
        }
        return output;
    }

    json writeFile(const json &input) {
        json output;
        if(!input.contains("path") || !input.contains("data")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        fs::FileWriterOptions fileWriterOptions;
        fileWriterOptions.filename = input["path"].get<string>();
        fileWriterOptions.data = input["data"].get<string>();
        if(fs::writeFile(fileWriterOptions))
            output["success"] = true;
        else
            output["error"] = helpers::makeErrorPayload("NE_FS_FILWRER", 
                                "Unable to write file: " + fileWriterOptions.filename);
        return output;
    }
    
    json writeBinaryFile(const json &input) {
        json output;
        if(!input.contains("path") || !input.contains("data")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        fs::FileWriterOptions fileWriterOptions;
        fileWriterOptions.filename = input["path"].get<string>();
        fileWriterOptions.data = base64::from_base64(input["data"].get<string>());
        if(fs::writeFile(fileWriterOptions))
            output["success"] = true;
        else
            output["error"] = helpers::makeErrorPayload("NE_FS_FILWRER", 
                                "Unable to write file: " + fileWriterOptions.filename);
        return output;
    }

    json removeFile(const json &input) {
        json output;
        if(!input.contains("path")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        string filename = input["path"].get<string>();
        if(fs::removeFile(filename)) {
            output["success"] = true;
            output["message"] = filename + " was deleted";
        }
        else{
            output["error"] = helpers::makeErrorPayload("NE_FS_FILRMER", 
                                "Cannot remove " + filename);
        }
        return output;
    }

    json readDirectory(const json &input) {
        json output;
        if(!input.contains("path")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        string path = input["path"].get<string>();
        fs::FileStats fileStats = fs::getStats(path);
        if(fileStats.hasError) {
            output["error"] = helpers::makeErrorPayload("NE_FS_NOPATHE", fileStats.error);
            return output;
        }
        
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
                output["returnValue"].push_back(file);
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
                output["returnValue"].push_back(file);
            } while(FindNextFile(hFind, &fd));
            FindClose(hFind);
            output["success"] = true;
        }
        #endif
        return output;
    }
   
    json copyFile(const json &input) {
        json output;
        if(!input.contains("source") || !input.contains("destination")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        string source = input["source"].get<string>();
        string destination = input["destination"].get<string>();
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
            output["error"] = helpers::makeErrorPayload("NE_FS_COPYFER",
                                "Cannot copy " + source + " to " + destination);
        }
        return output;
    } 
    
    json moveFile(const json &input) {
        json output;
        if(!input.contains("source") || !input.contains("destination")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        string source = input["source"].get<string>();
        string destination = input["destination"].get<string>();
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
            output["error"] = helpers::makeErrorPayload("NE_FS_MOVEFER",
                                "Cannot move " + source + " to " + destination);
        }
        return output;
    } 
    
    json getStats(const json &input) {
        json output;
        if(!input.contains("path")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        string path = input["path"].get<string>();
        fs::FileStats fileStats = fs::getStats(path);
        if(!fileStats.hasError) {
            json stats;
            stats["size"] = fileStats.size;
            stats["isFile"] = fileStats.isFile;
            stats["isDirectory"] = fileStats.isDirectory;
            output["returnValue"] = stats;
            output["success"] = true;
        }
        else{
            output["error"] = helpers::makeErrorPayload("NE_FS_NOPATHE", fileStats.error);
        }
        return output;
    } 
    
} // namespace controllers
} // namespace fs
