#ifndef NEU_FILESYSTEM_H
#define NEU_FILESYSTEM_H

#include <string>

#include "lib/json/json.hpp"

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
    bool writeFile(const fs::FileWriterOptions &fileWriterOptions);
    string getDirectoryName(string filename);
    string getCurrentDirectory();
    string getFullPathFromRelative(string path);
    fs::FileStats getStats(string path);

namespace controllers {
    json createDirectory(const json &input);
    json removeDirectory(const json &input);
    json writeFile(const json &input);
    json writeBinaryFile(const json &input);
    json readFile(const json &input);
    json readBinaryFile(const json &input);
    json removeFile(const json &input);
    json readDirectory(const json &input);
    json copyFile(const json &input);
    json moveFile(const json &input);
    json getStats(const json &input);
} // namespace controllers
} // namespace fs

#endif // #define NEU_FILESYSTEM_H
