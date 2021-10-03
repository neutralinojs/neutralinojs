#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "lib/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace fs {
    
    struct FileReaderResult {
        bool hasError = false;
        string error;
        string data;
    };
    
    struct FileWriterOptions {
        string filename;
        string data;
    };
    
    struct FileStats {
        bool hasError = false;
        string error;
        long long size;
        bool isDirectory;
        bool isFile;
    }; 

    bool createDirectory(string path);
    bool removeFile(string filename);
    fs::FileReaderResult readFile(string filename);
    bool writeFile(fs::FileWriterOptions fileWriterOptions);
    string getDirectoryName(string filename);
    string getCurrentDirectory();
    string getFullPathFromRelative(string path);
    fs::FileStats getStats(string path);

namespace controllers {
    json createDirectory(json input);
    json removeDirectory(json input);
    json writeFile(json input);
    json writeBinaryFile(json input);
    json readFile(json input);
    json readBinaryFile(json input);
    json removeFile(json input);
    json readDirectory(json input);
    json copyFile(json input);
    json moveFile(json input);
    json getStats(json input);
} // namespace controllers
} // namespace fs

#endif
