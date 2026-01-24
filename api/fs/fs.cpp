#include <map>
#include <mutex>
#include <iostream>
#include <fstream>
#include <regex>
#include <algorithm>
#include <cctype>
#include <filesystem>

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

#define NEU_WINDOWS_TICK 10000000
#define NEU_SEC_TO_UNIX_EPOCH 11644473600LL
#endif

#include <efsw/efsw.hpp>
#include "lib/json/json.hpp"
#include "lib/base64/base64.hpp"
#include "lib/platformfolders/platform_folders.h"
#include "settings.h"
#include "helpers.h"
#include "errors.h"
#include "api/fs/fs.h"
#include "api/os/os.h"
#include "api/events/events.h"

using namespace std;
using json = nlohmann::json;

map<int, ifstream*> openedFiles;
mutex openedFilesLock;
efsw::FileWatcher* fileWatcher;
map<efsw::WatchID, pair<efsw::FileWatchListener*, string>> watchListeners;
mutex watcherLock;

#define NEU_DEFAULT_STREAM_BUF_SIZE 256

namespace fs {

void __dispatchOpenedFileEvt(int virtualFileId, const string &action, const json &data) {
    json evt;
    evt["id"] = virtualFileId;
    evt["action"] = action;
    evt["data"] = data;
    events::dispatch("openedFile", evt);
}

void __dispatchWatcherEvt(efsw::WatchID watcherId, const std::string& dir,
                           const std::string& filename, efsw::Action action,
                           std::string oldFilename) {
    json evt;
    string dirC = dir;
    evt["id"] = watcherId;
    evt["dir"] = helpers::normalizePath(dirC);
    evt["filename"] = filename;
    evt["timestamp"] = helpers::getCurrentTimestamp();
    switch (action) {
        case efsw::Actions::Add:
            evt["action"] = "add";
            break;
        case efsw::Actions::Delete:
            evt["action"] = "delete";
            break;
        case efsw::Actions::Modified:
            evt["action"] = "modified";
            break;
        case efsw::Actions::Moved:
            evt["action"] = "moved";
            evt["oldFilename"] = oldFilename;
            break;
    }
    events::dispatch("watchFile", evt);
}

void __readStreamBlock(const OpenedFileEvent &evt, ifstream *reader, vector<char> &buffer) {
    long long size = evt.size > -1 ? evt.size : NEU_DEFAULT_STREAM_BUF_SIZE;

    buffer.clear();
    buffer.resize(size);
    reader->read(buffer.data(), size);
    buffer.resize(reader->gcount());
    string result(buffer.begin(), buffer.end());

    if(regex_match(evt.type, regex(".*Binary$"))) {
        __dispatchOpenedFileEvt(evt.id, "dataBinary", base64::to_base64(result));
    }
    else {
        __dispatchOpenedFileEvt(evt.id, "data", result);
    }
}

void __readStream(const OpenedFileEvent &evt, ifstream *reader) {
    if(reader->eof()) {
        __dispatchOpenedFileEvt(evt.id, "end", nullptr);
        return;
    }

    vector<char> buffer;
    while(reader->peek() != char_traits<char>::eof()) {
        __readStreamBlock(evt, reader, buffer);

        if(evt.type == "read" || evt.type == "readBinary") {
            break;
        }
    }

    if(reader->eof()) {
        __dispatchOpenedFileEvt(evt.id, "end", nullptr);
    }
}

#if defined(_WIN32)
// From: https://stackoverflow.com/a/6161842/3565513
long long __winTickToUnixMS(long long windowsTicks) {
    return (windowsTicks / NEU_WINDOWS_TICK - NEU_SEC_TO_UNIX_EPOCH) * 1000;
}
#endif

string getDirectoryName(const string &filename){
    #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
    return dirname((char*)filename.c_str());
    #elif defined(_WIN32)
    wstring wideFilename = helpers::str2wstr(filename);
    LPWSTR pathToReplace = &wideFilename[0];
    PathRemoveFileSpec(pathToReplace);
    return helpers::normalizePath(helpers::wcstr2str(pathToReplace));
    #endif
}

string getCurrentDirectory() {
    string path = FS_CONVWSTR(filesystem::current_path());
    #if defined(_WIN32)
    return helpers::normalizePath(path);
    #else
    return path;
    #endif
}

fs::FileReaderResult readFile(const string &filename, const fs::FileReaderOptions &fileReaderOptions) {
    fs::FileReaderResult fileReaderResult;
    ifstream reader(CONVSTR(filename), ios::binary | ios::ate);
    if(!reader.is_open()) {
        fileReaderResult.status = errors::NE_FS_FILRDER;
        return fileReaderResult;
    }
    vector<char> buffer;
    long long origSize = reader.tellg();
    long long size = origSize;
    long long pos = 0;

    if(fileReaderOptions.pos > -1) {
        pos = min(fileReaderOptions.pos, origSize);
        size = origSize - pos;
    }
    if(fileReaderOptions.size > -1) {
        size = min(fileReaderOptions.size, size);
    }
    reader.seekg(pos, ios::beg);

    buffer.resize(size);
    reader.read(buffer.data(), size);
    string result(buffer.begin(), buffer.end());
    reader.close();

    fileReaderResult.data = result;
    return fileReaderResult;
}

bool writeFile(const fs::FileWriterOptions &fileWriterOptions) {
    json output;
    ios_base::openmode mode = ios_base::out | ios_base::binary;
    // For portability we use LF ('\n') always via ::binary
    if(fileWriterOptions.append) {
        mode |= ios_base::app;
    }

    ofstream writer(CONVSTR(fileWriterOptions.filename), mode);
    if(!writer.is_open())
        return false;
    writer << fileWriterOptions.data;
    writer.close();
    return true;
}

int openFile(const string &filename) {
    int virtualFileId = openedFiles.size();
    ifstream *reader = new ifstream(CONVSTR(filename), ios::binary);
    if(!reader->is_open()) {
        delete reader;
        return -1;
    }
    lock_guard<mutex> guard(openedFilesLock);
    openedFiles[virtualFileId] = reader;
    return virtualFileId;
}

bool updateOpenedFile(const OpenedFileEvent &evt) {
    lock_guard<mutex> guard(openedFilesLock);
    if(openedFiles.find(evt.id) == openedFiles.end()) {
        return false;
    }
    ifstream *reader = openedFiles[evt.id];

    if(evt.type == "close") {
        reader->close();
        delete reader;
        openedFiles.erase(evt.id);
    }
    else if(evt.type == "read" || evt.type == "readAll"
            || evt.type == "readBinary" || evt.type == "readAllBinary") {
        __readStream(evt, reader);
    }
    else if(evt.type == "seek") {
        long long pos = evt.pos > -1 ? evt.pos : 0;
        reader->clear();
        reader->seekg(pos, ios::beg);
    }
    else {
        return false;
    }
    return true;
}

class __WatcherListener: public efsw::FileWatchListener {
  public:
    void handleFileAction( efsw::WatchID watcherId, const std::string& dir,
                           const std::string& filename, efsw::Action action,
                           std::string oldFilename ) override {
        __dispatchWatcherEvt(watcherId, dir, filename, action, oldFilename);
    }
};

long createWatcher(const string &path) {
    lock_guard<mutex> guard(watcherLock);
    if(fileWatcher == nullptr) {
        fileWatcher = new efsw::FileWatcher();
    }

    for(const auto &[wid, info]: watchListeners) {
        if(info.second == path) {
            return (long)wid;
        }
    }

    __WatcherListener* listener = new __WatcherListener();
    efsw::WatchID watcherId = fileWatcher->addWatch(path, listener, true);
    watchListeners[watcherId] = make_pair(listener, path);
    fileWatcher->watch();
    return (long)watcherId;
}

bool removeWatcher(long watcherId) {
    lock_guard<mutex> guard(watcherLock);
    if(fileWatcher == nullptr || watchListeners.find(watcherId) == watchListeners.end()) {
        return false;
    }
    fileWatcher->removeWatch(watcherId);
    delete watchListeners[watcherId].first;
    watchListeners.erase(watcherId);
    return true;
}

fs::FileStats getStats(const string &path) {
    fs::FileStats fileStats;
    #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
    struct stat statBuffer;
    if(stat(path.c_str(), &statBuffer) == 0) {
        fileStats.size = statBuffer.st_size;
        if(S_ISREG(statBuffer.st_mode)) {
            fileStats.entryType = fs::EntryTypeFile;
        }
        if(S_ISDIR(statBuffer.st_mode)) {
            fileStats.entryType = fs::EntryTypeDir;
        }
        fileStats.createdAt = statBuffer.st_ctime * 1000;
        fileStats.modifiedAt = statBuffer.st_mtime * 1000;
    }

    #elif defined(_WIN32)
    HANDLE hFile = CreateFile(helpers::str2wstr(path).c_str(), GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);

    LARGE_INTEGER size;
    FILE_BASIC_INFO basicInfo;
    if(GetFileSizeEx(hFile, &size) &&
        GetFileInformationByHandleEx(hFile, FileBasicInfo, &basicInfo, sizeof(basicInfo))
    ) {
        fileStats.size = size.QuadPart;
        if(!(basicInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            fileStats.entryType = fs::EntryTypeFile;
        }
        if(basicInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            fileStats.entryType = fs::EntryTypeDir;
        }
        fileStats.createdAt = __winTickToUnixMS(basicInfo.CreationTime.QuadPart);
        fileStats.modifiedAt = __winTickToUnixMS(basicInfo.ChangeTime.QuadPart);
    }

    #endif
    else {
        fileStats.status = errors::NE_FS_NOPATHE;
    }
    #if defined(_WIN32)
    CloseHandle(hFile);
    #endif
    return fileStats;
}

fs::DirReaderResult readDirectory(const string &path, bool recursive) {
    fs::DirReaderResult dirResult;
    fs::FileStats fileStats = fs::getStats(path);
    if(fileStats.status != errors::NE_ST_OK) {
        dirResult.status = fileStats.status;
        return dirResult;
    }

    for(auto entry = filesystem::recursive_directory_iterator(CONVSTR(path));
        entry != filesystem::recursive_directory_iterator();
        ++entry) {

        fs::EntryType type = fs::EntryTypeOther;
        if(entry->is_directory()) {
            type = fs::EntryTypeDir;
        }
        else if(entry->is_regular_file()) {
            type = fs::EntryTypeFile;
        }

        auto entryPath = entry->path();
        string entryStr = FS_CONVWSTR(entry->path());

        dirResult.entries.push_back({ FS_CONVWSTR(entryPath.filename()),
            helpers::normalizePath(entryStr), type });

        if(!recursive) {
            entry.disable_recursion_pending();
        }
    }
    return dirResult;
}

string applyPathConstants(const string &path) {
    string newPath = regex_replace(path, regex("\\$\\{NL_PATH\\}"), settings::getAppPath());

    vector<string> pathNames = {"data", "cache", "documents", 
                    "pictures", "music", "video", "downloads",
                    "saveGames1", "saveGames2", "temp"};
    for(const string &pathName: pathNames) {
        string varSegment = pathName;
        transform(varSegment.begin(), varSegment.end(), varSegment.begin(), ::toupper); 
        newPath = regex_replace(newPath, regex("\\$\\{NL_OS" + varSegment + "PATH\\}"), os::getPath(pathName));
    }
    return newPath;
}

namespace controllers {

json __writeOrAppendFile(const json &input, bool append = false) {
    json output;
    const auto missingRequiredField = helpers::missingRequiredField(input, {"path", "data"});
    if(missingRequiredField) {
        output["error"] = errors::makeMissingArgErrorPayload(missingRequiredField.value());
        return output;
    }
    fs::FileWriterOptions fileWriterOptions;
    fileWriterOptions.filename = input["path"].get<string>();
    fileWriterOptions.data = input["data"].get<string>();
    fileWriterOptions.append = append;

    if(fs::writeFile(fileWriterOptions))
        output["success"] = true;
    else
        output["error"] = errors::makeErrorPayload(errors::NE_FS_FILWRER, fileWriterOptions.filename);
    return output;
}

json __writeOrAppendBinaryFile(const json &input, bool append = false) {
    json output;
    const auto missingRequiredField = helpers::missingRequiredField(input, {"path", "data"});
    if(missingRequiredField) {
        output["error"] = errors::makeMissingArgErrorPayload(missingRequiredField.value());
        return output;
    }
    fs::FileWriterOptions fileWriterOptions;
    fileWriterOptions.filename = input["path"].get<string>();
    fileWriterOptions.data = base64::from_base64(input["data"].get<string>());
    fileWriterOptions.append = append;

    if(fs::writeFile(fileWriterOptions))
        output["success"] = true;
    else
        output["error"] = errors::makeErrorPayload(errors::NE_FS_FILWRER, fileWriterOptions.filename);
    return output;
}

json createDirectory(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload("path");
        return output;
    }
    string path = input["path"].get<string>();
    if(filesystem::create_directories(CONVSTR(path))) {
        output["success"] = true;
        output["message"] = "Directory " + path + " was created";
    }
    else {
        output["error"] = errors::makeErrorPayload(errors::NE_FS_DIRCRER, path);
    }
    return output;
}

json remove(const json& input) {
    json output;

    if (!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload("path");
        return output;
    }

    std::string path = input["path"].get<std::string>();

    if(filesystem::remove_all(CONVSTR(path))) {
        output["success"] = true;
        output["message"] = path + " was removed";
    }
    else {
        output["error"] = errors::makeErrorPayload(errors::NE_FS_REMVERR, path);
    }

    return output;
}

json readFile(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload("path");
        return output;
    }
    fs::FileReaderOptions readerOptions;
    if(helpers::hasField(input, "pos")) {
        readerOptions.pos = input["pos"].get<long long>();
    }
    if(helpers::hasField(input, "size")) {
        readerOptions.size = input["size"].get<long long>();
    }
    string path = input["path"].get<string>();
    fs::FileReaderResult fileReaderResult;
    fileReaderResult = fs::readFile(path, readerOptions);
    if(fileReaderResult.status != errors::NE_ST_OK) {
        output["error"] = errors::makeErrorPayload(fileReaderResult.status, path);
    }
    else {
        output["returnValue"] = fileReaderResult.data;
        output["success"] = true;
    }
    return output;
}

json readBinaryFile(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload("path");
        return output;
    }
    fs::FileReaderOptions readerOptions;
    if(helpers::hasField(input, "pos")) {
        readerOptions.pos = input["pos"].get<long long>();
    }
    if(helpers::hasField(input, "size")) {
        readerOptions.size = input["size"].get<long long>();
    }
    string path = input["path"].get<string>();
    fs::FileReaderResult fileReaderResult;
    fileReaderResult = fs::readFile(path, readerOptions);
    if(fileReaderResult.status != errors::NE_ST_OK) {
        output["error"] = errors::makeErrorPayload(fileReaderResult.status, path);
    }
    else {
        output["returnValue"] = base64::to_base64(fileReaderResult.data);
        output["success"] = true;
    }
    return output;
}

json writeFile(const json &input) {
    return __writeOrAppendFile(input);
}

json appendFile(const json &input) {
    return __writeOrAppendFile(input, true);
}

json writeBinaryFile(const json &input) {
    return __writeOrAppendBinaryFile(input);
}

json appendBinaryFile(const json &input) {
    return __writeOrAppendBinaryFile(input, true);
}

json openFile(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload("path");
        return output;
    }
    string path = input["path"].get<string>();
    int fileId = fs::openFile(path);
    if(fileId == -1) {
        output["error"] = errors::makeErrorPayload(errors::NE_FS_FILOPER, path);
    }
    else {
        output["returnValue"] = fileId;
        output["success"] = true;
    }
    return output;
}

json updateOpenedFile(const json &input) {
    json output;
    const auto missingRequiredField = helpers::missingRequiredField(input, {"id", "event"});
    if(missingRequiredField) {
        output["error"] = errors::makeMissingArgErrorPayload(missingRequiredField.value());
        return output;
    }

    fs::OpenedFileEvent fileEvt;
    fileEvt.id = input["id"].get<int>();
    fileEvt.type = input["event"].get<string>();

    if(helpers::hasField(input, "data")) {
        if(fileEvt.type == "read" || fileEvt.type == "readAll"
            || fileEvt.type == "readBinary" || fileEvt.type == "readAllBinary") {
            fileEvt.size = input["data"].get<long long>();
        }
        else if(fileEvt.type == "seek") {
            fileEvt.pos = input["data"].get<long long>();
        }
    }

    if(fs::updateOpenedFile(fileEvt)) {
        output["success"] = true;
    }
    else {
        output["error"] = errors::makeErrorPayload(errors::NE_FS_UNLTOUP, to_string(fileEvt.id));
    }
    return output;
}

json getOpenedFileInfo(const json &input) {
    json output;
    lock_guard<mutex> guard(openedFilesLock);

    if(!helpers::hasRequiredFields(input, {"id"})) {
        output["error"] = errors::makeMissingArgErrorPayload("id");
        return output;
    }
    int fileId = input["id"].get<int>();

    if(openedFiles.find(fileId) == openedFiles.end()) {
        output["error"] = errors::makeErrorPayload(errors::NE_FS_UNLTFOP, to_string(fileId));
        return output;
    }
    ifstream *reader = openedFiles[fileId];
    long long pos = reader->tellg();

    json file;
    file["id"] = fileId;
    file["eof"] = reader->eof();
    file["pos"] = pos;
    file["lastRead"] = reader->gcount();

    output["returnValue"] = file;
    output["success"] = true;
    return output;
}

json readDirectory(const json &input) {
    json output;
    output["returnValue"] = json::array();
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload("path");
        return output;
    }
    string path = input["path"].get<string>();
    bool recursive = false;

    if(helpers::hasField(input, "recursive")) {
        recursive = input["recursive"].get<bool>();
    }

    fs::DirReaderResult dirResult = fs::readDirectory(path, recursive);
    if(dirResult.status != errors::NE_ST_OK) {
        output["error"] = errors::makeErrorPayload(dirResult.status, path);
        return output;
    }

    for(const fs::DirReaderEntry &entry: dirResult.entries) {
        string type = "OTHER";

        if(entry.type == fs::EntryTypeDir) {
            type = "DIRECTORY";
        }
        else if(entry.type == fs::EntryTypeFile) {
            type = "FILE";
        }

        output["returnValue"].push_back({
            {"entry", entry.name},
            {"path", entry.path},
            {"type", type},
        });
    }
    output["success"] = true;
    return output;
}

json copy(const json &input) {
    json output;
    const auto missingRequiredField = helpers::missingRequiredField(input, {"source", "destination"});
    if(missingRequiredField) {
        output["error"] = errors::makeMissingArgErrorPayload(missingRequiredField.value());
        return output;
    }
    string source = input["source"].get<string>();
    string destination = input["destination"].get<string>();

    error_code ec;
    auto copyOptions = filesystem::copy_options::none;

    if(!helpers::hasField(input, "recursive") || input["recursive"].get<bool>()) {
        copyOptions |= filesystem::copy_options::recursive;
    }

    if(!helpers::hasField(input, "overwrite") || input["overwrite"].get<bool>()) {
        copyOptions |= filesystem::copy_options::overwrite_existing;
    }

    if(helpers::hasField(input, "skip") && input["skip"].get<bool>()) {
        copyOptions |= filesystem::copy_options::skip_existing;
    }

    filesystem::copy(CONVSTR(source), CONVSTR(destination), copyOptions, ec);

    if(!ec) {
        output["success"] = true;
        output["message"] = "Copy operation was successful";
    }
    else{
        output["error"] = errors::makeErrorPayload(errors::NE_FS_COPYERR, source + " -> " + destination);
    }
    return output;
}

json move(const json &input) {
    json output;
    const auto missingRequiredField = helpers::missingRequiredField(input, {"source", "destination"});
    if(missingRequiredField) {
        output["error"] = errors::makeMissingArgErrorPayload(missingRequiredField.value());
        return output;
    }
    string source = input["source"].get<string>();
    string destination = input["destination"].get<string>();

    error_code ec;
    filesystem::rename(CONVSTR(source), CONVSTR(destination), ec);

    if(!ec) {
        output["success"] = true;
        output["message"] = "File move operation was successful";
    }
    else{
        output["error"] = errors::makeErrorPayload(errors::NE_FS_MOVEERR, source + " -> " + destination);
    }
    return output;
}

json getStats(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload("path");
        return output;
    }
    string path = input["path"].get<string>();
    fs::FileStats fileStats = fs::getStats(path);
    if(fileStats.status == errors::NE_ST_OK) {
        json stats;
        stats["size"] = fileStats.size;
        stats["isFile"] = fileStats.entryType == fs::EntryTypeFile;
        stats["isDirectory"] = fileStats.entryType == fs::EntryTypeDir;
        stats["createdAt"] = fileStats.createdAt;
        stats["modifiedAt"] = fileStats.modifiedAt;
        output["returnValue"] = stats;
        output["success"] = true;
    }
    else{
        output["error"] = errors::makeErrorPayload(fileStats.status, path);
    }
    return output;
}

json createWatcher(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload("path");
        return output;
    }
    string path = input["path"].get<string>();
    long watcherId = fs::createWatcher(path);

    if(watcherId <= 0) {
        output["error"] = errors::makeErrorPayload(errors::NE_FS_UNLCWAT, path);
    }
    else {
        output["returnValue"] = watcherId;
        output["success"] = true;
    }
    return output;
}


json removeWatcher(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"id"})) {
        output["error"] = errors::makeMissingArgErrorPayload("id");
        return output;
    }
    long watcherId = input["id"].get<long>();
    if(!fs::removeWatcher(watcherId)) {
        output["error"] = errors::makeErrorPayload(errors::NE_FS_NOWATID, to_string(watcherId));
    }
    else {
        output["returnValue"] = watcherId;
        output["success"] = true;
    }
    return output;
}

json getWatchers(const json &input) {
    json output;
    output["returnValue"] = json::array();
    for(const auto &[watcherId, info]: watchListeners) {
        output["returnValue"].push_back({
            {"id", watcherId},
            {"path", info.second}
        });
    }
    output["success"] = true;
    return output;
}

json getAbsolutePath(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload("path");
        return output;
    }
    string path = input["path"].get<string>();
    string absPath = FS_CONVWSTRN(filesystem::absolute(path));
    output["returnValue"] = absPath;
    output["success"] = true;
    return output;
}

json getRelativePath(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload("path");
        return output;
    }
    string path = input["path"].get<string>();
    string base = filesystem::current_path().string();

    if(helpers::hasField(input, "base")) {
        base = input["base"].get<string>();
    }
    
    string relPath = FS_CONVWSTRN(filesystem::relative(CONVSTR(path), CONVSTR(base)));
    output["returnValue"] = relPath;
    output["success"] = true;
    return output;
}

json getPathParts(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload("path");
        return output;
    }
    string path = input["path"].get<string>();
    auto pathObj = filesystem::path(CONVSTR(path));
    
    json pathParts = {
        {"rootName", FS_CONVWSTRN(pathObj.root_name())},
        {"rootDirectory", FS_CONVWSTRN(pathObj.root_directory())},
        {"rootPath", FS_CONVWSTRN(pathObj.root_path())},
        {"relativePath", FS_CONVWSTRN(pathObj.relative_path())},
        {"parentPath", FS_CONVWSTRN(pathObj.parent_path())},
        {"filename", pathObj.filename()},
        {"stem", pathObj.stem()},
        {"extension", pathObj.extension()}
    };
    output["returnValue"] = pathParts;
    output["success"] = true;
    return output;
}

json getPermissions(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload("path");
        return output;
    }
    string path = input["path"].get<string>();
    
    fs::FileStats fileStats = fs::getStats(path);
    if(fileStats.status != errors::NE_ST_OK) {
        output["error"] = errors::makeErrorPayload(fileStats.status, path);
        return output;
    }
    auto perms = filesystem::status(CONVSTR(path)).permissions();
    
    json permissions = {
        {"all", filesystem::perms::all == (perms & filesystem::perms::all)},
        {"ownerAll", filesystem::perms::owner_all == (perms & filesystem::perms::owner_all)},
        {"ownerRead", filesystem::perms::none != (perms & filesystem::perms::owner_read)},
        {"ownerWrite", filesystem::perms::none != (perms & filesystem::perms::owner_write)},
        {"ownerExec", filesystem::perms::none != (perms & filesystem::perms::owner_exec)},
        {"groupAll", filesystem::perms::group_all == (perms & filesystem::perms::group_all)},
        {"groupRead", filesystem::perms::none != (perms & filesystem::perms::group_read)},
        {"groupWrite", filesystem::perms::none != (perms & filesystem::perms::group_write)},
        {"groupExec", filesystem::perms::none != (perms & filesystem::perms::group_exec)},
        {"othersAll", filesystem::perms::others_all == (perms & filesystem::perms::others_all)},
        {"othersRead", filesystem::perms::none != (perms & filesystem::perms::others_read)},
        {"othersWrite", filesystem::perms::none != (perms & filesystem::perms::others_write)},
        {"othersExec", filesystem::perms::none != (perms & filesystem::perms::others_exec)}
    };
    
    output["returnValue"] = permissions;
    output["success"] = true;
    return output;
}

json setPermissions(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload("path");
        return output;
    }
    string path = input["path"].get<string>();
    
    error_code ec;
    filesystem::perms permissions = filesystem::perms::none;
    filesystem::perm_options permMode = filesystem::perm_options::replace;

    if(helpers::hasField(input, "all") && input["all"].get<bool>())
        permissions |= filesystem::perms::all;
    if(helpers::hasField(input, "ownerAll") && input["ownerAll"].get<bool>())
        permissions |= filesystem::perms::owner_all;
    if(helpers::hasField(input, "groupAll") && input["groupAll"].get<bool>())
        permissions |= filesystem::perms::group_all;
    if(helpers::hasField(input, "othersAll") && input["othersAll"].get<bool>())
        permissions |= filesystem::perms::others_all;
    if(helpers::hasField(input, "ownerRead") && input["ownerRead"].get<bool>())
        permissions |= filesystem::perms::owner_read;
    if(helpers::hasField(input, "ownerWrite") && input["ownerWrite"].get<bool>())
        permissions |= filesystem::perms::owner_write;
    if(helpers::hasField(input, "ownerExec") && input["ownerExec"].get<bool>())
        permissions |= filesystem::perms::owner_exec;
    if(helpers::hasField(input, "groupRead") && input["groupRead"].get<bool>())
        permissions |= filesystem::perms::group_read;
    if(helpers::hasField(input, "groupWrite") && input["groupWrite"].get<bool>())
        permissions |= filesystem::perms::group_write;
    if(helpers::hasField(input, "groupExec") && input["groupExec"].get<bool>())
        permissions |= filesystem::perms::group_exec;
    if(helpers::hasField(input, "othersRead") && input["othersRead"].get<bool>())
        permissions |= filesystem::perms::others_read;
    if(helpers::hasField(input, "othersWrite") && input["othersWrite"].get<bool>())
        permissions |= filesystem::perms::others_write;
    if(helpers::hasField(input, "othersExec") && input["othersExec"].get<bool>())
        permissions |= filesystem::perms::others_exec;

    if(helpers::hasField(input, "mode")) {
        string mode = input["mode"].get<string>();
        if(mode == "ADD") permMode = filesystem::perm_options::add;
        if(mode == "REPLACE") permMode = filesystem::perm_options::replace;
        if(mode == "REMOVE") permMode = filesystem::perm_options::remove;
    }
    
    filesystem::permissions(CONVSTR(path), permissions, permMode, ec);

    if(!ec) { 
        output["returnValue"] = permissions;
        output["success"] = true;
    }
    else {
        output["error"] = errors::makeErrorPayload(errors::NE_FS_UNLSTPR, path);
    }
    return output;
}

json getJoinedPath(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"paths"})) {
        output["error"] = errors::makeMissingArgErrorPayload("paths");
        return output;
    }
    vector<string> paths = input["paths"].get<vector<string>>();
    filesystem::path joinedPath = "";

    for(const string &path: paths) {
        joinedPath /= filesystem::path(CONVSTR(path));
    }
    
    output["returnValue"] = FS_CONVWSTRN(filesystem::weakly_canonical(joinedPath));
    output["success"] = true;
    return output;
}

json getNormalizedPath(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload("path");
        return output;
    }
    string path = input["path"].get<string>();
    
    output["returnValue"] = helpers::normalizePath(path);
    output["success"] = true;
    return output;
}

json getUnnormalizedPath(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload("path");
        return output;
    }
    string path = input["path"].get<string>();
    
    output["returnValue"] = helpers::unNormalizePath(path);
    output["success"] = true;
    return output;
}

} // namespace controllers

} // namespace fs
