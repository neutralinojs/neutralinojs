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
    string path;
    fs::EntryType type = fs::EntryTypeOther;
};

struct DirReaderResult {
    errors::StatusCode status = errors::NE_ST_OK;
    vector<DirReaderEntry> entries;
};

fs::FileReaderResult readFile(const string &filename, const fs::FileReaderOptions &fileReaderOptions = {});
bool writeFile(const fs::FileWriterOptions &fileWriterOptions);
string getDirectoryName(const string &filename);
string getCurrentDirectory();
int openFile(const string &path);
bool updateOpenedFile(const OpenedFileEvent &evt);
long createWatcher(const string &path);
bool removeWatcher(long watcherId);
fs::FileStats getStats(const string &path);
fs::DirReaderResult readDirectory(const string &path, bool recursive = false);
string applyPathConstants(const string &path);

namespace controllers {

json createDirectory(const json &input);
json remove(const json &input);
json writeFile(const json &input);
json writeBinaryFile(const json &input);
json appendFile(const json &input);
json appendBinaryFile(const json &input);
json readFile(const json &input);
json readBinaryFile(const json &input);
json openFile(const json &input);
json updateOpenedFile(const json &input);
json getOpenedFileInfo(const json &input);
json readDirectory(const json &input);
json copy(const json &input);
json move(const json &input);
json getStats(const json &input);
json createWatcher(const json &input);
json removeWatcher(const json &input);
json getWatchers(const json &input);
json getAbsolutePath(const json &input);
json getRelativePath(const json &input);
json getPathParts(const json &input);
json getPermissions(const json &input);
json setPermissions(const json &input);
json getJoinedPath(const json &input);
json getNormalizedPath(const json &input);
json getUnnormalizedPath(const json &input);

} // namespace controllers

} // namespace fs

#endif // #define NEU_FILESYSTEM_H
