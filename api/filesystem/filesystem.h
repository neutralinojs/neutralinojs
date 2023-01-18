#ifndef NEU_FILESYSTEM_H
#define NEU_FILESYSTEM_H

#include <string>
#include <vector>

#include "errors.h"
#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace fs {

struct FileReaderResult {
    errors::StatusCode status = errors::NE_ST_OK;
    string data;
};

struct FileReaderOptions {
    long long pos = -1;
    long long size = -1;
};

struct FileWriterOptions {
    string filename;
    string data;
    bool append = false;
};

struct OpenedFileEvent {
    int id = -1;
    string type = "";
    long long pos = -1;
    long long size = -1;
};

enum EntryType { EntryTypeFile, EntryTypeDir, EntryTypeOther };

struct FileStats {
    errors::StatusCode status = errors::NE_ST_OK;
    long long size;
    fs::EntryType entryType = fs::EntryTypeOther;
    long long createdAt;
    long long modifiedAt;
};

struct DirReaderEntry {
    string name;
    fs::EntryType type = fs::EntryTypeOther;
};

struct DirReaderResult {
    errors::StatusCode status = errors::NE_ST_OK;
    vector<DirReaderEntry> entries;
};

bool createDirectory(const string &path);
bool removeFile(const string &filename);
fs::FileReaderResult readFile(const string &filename, const fs::FileReaderOptions &fileReaderOptions = {});
bool writeFile(const fs::FileWriterOptions &fileWriterOptions);
string getDirectoryName(const string &filename);
string getCurrentDirectory();
string getFullPathFromRelative(const string &path);
int openFile(const string &path);
bool updateOpenedFile(const OpenedFileEvent &evt);
fs::FileStats getStats(const string &path);
fs::DirReaderResult readDirectory(const string &path);

namespace controllers {

json createDirectory(const json &input);
json removeDirectory(const json &input);
json writeFile(const json &input);
json writeBinaryFile(const json &input);
json appendFile(const json &input);
json appendBinaryFile(const json &input);
json readFile(const json &input);
json readBinaryFile(const json &input);
json openFile(const json &input);
json updateOpenedFile(const json &input);
json getOpenedFileInfo(const json &input);
json removeFile(const json &input);
json readDirectory(const json &input);
json copyFile(const json &input);
json moveFile(const json &input);
json getStats(const json &input);

} // namespace controllers

} // namespace fs

#endif // #define NEU_FILESYSTEM_H
