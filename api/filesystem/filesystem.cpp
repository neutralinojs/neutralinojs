#include <map>
#include <mutex>
#include <iostream>
#include <fstream>

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>

#define CONVSTR(S) S.c_str()
#elif defined(_WIN32)
#define _WINSOCKAPI_
#include <windows.h>
#include <atlstr.h>
#include <shlwapi.h>
#include <winbase.h>

#define NEU_WINDOWS_TICK 10000000
#define NEU_SEC_TO_UNIX_EPOCH 11644473600LL

// ifstream and ofstream do not support UTF-8 file paths on Windows.
// However there is a non-standard extension which allows the use of wide strings.
// So, before we pass the path string to the constructor, we have to convert it to a UTF-16 std::wstring.
#define CONVSTR(S) helpers::str2wstr(S)
#endif

#include <efsw/efsw.hpp>
#include "lib/json/json.hpp"
#include "lib/base64/base64.hpp"
#include "settings.h"
#include "helpers.h"
#include "errors.h"
#include "api/filesystem/filesystem.h"
#include "api/os/os.h"
#include "api/events/events.h"

using namespace std;
using json = nlohmann::json;

map<int, ifstream*> openedFiles;
mutex openedFilesLock;
efsw::FileWatcher* fileWatcher;
map<efsw::WatchID, efsw::FileWatchListener*> watchListeners;
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
    string result(buffer.begin(), buffer.end());

    __dispatchOpenedFileEvt(evt.id, "data", result);
}

void __readStream(const OpenedFileEvent &evt, ifstream *reader) {
    if(reader->eof()) {
        __dispatchOpenedFileEvt(evt.id, "end", nullptr);
        return;
    }

    vector<char> buffer;
    while(reader->peek() != char_traits<char>::eof()) {
        __readStreamBlock(evt, reader, buffer);

        if(evt.type == "read") {
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

bool createDirectory(const string &path) {
    #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
    return mkdir(path.c_str(), 0700) == 0;
    #elif defined(_WIN32)
    return CreateDirectory(helpers::str2wstr(path).c_str(), nullptr) == 1;
    #endif
}

bool removeFile(const string &filename) {
    #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
    return remove(filename.c_str()) == 0;
    #elif defined(_WIN32)
    return DeleteFile(helpers::str2wstr(filename).c_str()) == 1;
    #endif
}

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
    #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
    return getcwd(nullptr, 0);
    #elif defined(_WIN32)
    TCHAR currentDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentDir);
    string currentDirStr(helpers::wstr2str(currentDir));
    return helpers::normalizePath(currentDirStr);
    #endif
}

string getFullPathFromRelative(const string &path) {
    #if defined(__linux__)
    return realpath(path.c_str(), nullptr);
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
    else if(evt.type == "read" || evt.type == "readAll") {
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
    __WatcherListener* listener = new __WatcherListener();
    efsw::WatchID watcherId = fileWatcher->addWatch(path, listener, true);
    watchListeners[watcherId] = listener;
    fileWatcher->watch();
    return (long)watcherId;
}

bool removeWatcher(long watcherId) {
    lock_guard<mutex> guard(watcherLock);
    if(fileWatcher == nullptr || watchListeners.find(watcherId) == watchListeners.end()) {
        return false;
    }
    fileWatcher->removeWatch(watcherId);
    delete watchListeners[watcherId];
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

fs::DirReaderResult readDirectory(const string &path) {
    fs::DirReaderResult dirResult;
    fs::FileStats fileStats = fs::getStats(path);
    if(fileStats.status != errors::NE_ST_OK) {
        dirResult.status = fileStats.status;
        return dirResult;
    }

    #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
    DIR *dirp;
    struct dirent *directory;
    dirp = opendir(path.c_str());
    if(dirp) {
        while((directory = readdir(dirp)) != nullptr) {
            fs::EntryType type = fs::EntryTypeOther;
            if(directory->d_type == DT_DIR) {
                type = fs::EntryTypeDir;
            }
            else if(directory->d_type == DT_REG) {
                type = fs::EntryTypeFile;
            }

            dirResult.entries.push_back({ directory->d_name, type });
        }
        closedir(dirp);
    }
    #elif defined(_WIN32)
    string search_path = path + "/*.*";
    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFile(helpers::str2wstr(search_path).c_str(), &fd);
    if(hFind != INVALID_HANDLE_VALUE) {
        do {
            fs::EntryType type = fs::EntryTypeOther;
            if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
                type = fs::EntryTypeDir;
            }
            else {
                type = fs::EntryTypeFile;
            }

            dirResult.entries.push_back({ helpers::wstr2str(fd.cFileName), type });
        } while(FindNextFile(hFind, &fd));
        FindClose(hFind);
    }
    #endif
    return dirResult;
}

namespace controllers {

json __writeOrAppendFile(const json &input, bool append = false) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path", "data"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
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
    if(!helpers::hasRequiredFields(input, {"path", "data"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
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
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    string path = input["path"].get<string>();
    if(fs::createDirectory(path)) {
        output["success"] = true;
        output["message"] = "Directory " + path + " was created";
    }
    else{
        output["error"] = errors::makeErrorPayload(errors::NE_FS_DIRCRER, path);
    }
    return output;
}

json removeDirectory(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    string path = input["path"].get<string>();
    #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
    if(rmdir(path.c_str()) == 0) {
    #elif defined(_WIN32)
    if(RemoveDirectory(helpers::str2wstr(path).c_str())) {
    #endif
        output["success"] = true;
        output["message"] = "Directory " + path + " was removed";
    }
    else{
        output["error"] = errors::makeErrorPayload(errors::NE_FS_RMDIRER, path);
    }
    return output;
}

json readFile(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
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
        output["error"] = errors::makeMissingArgErrorPayload();
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
        output["error"] = errors::makeMissingArgErrorPayload();
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
    if(!helpers::hasRequiredFields(input, {"id", "event"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }

    fs::OpenedFileEvent fileEvt;
    fileEvt.id = input["id"].get<int>();
    fileEvt.type = input["event"].get<string>();

    if(helpers::hasField(input, "data")) {
        if(fileEvt.type == "read" || fileEvt.type == "readAll") {
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
        output["error"] = errors::makeMissingArgErrorPayload();
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

json removeFile(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    string filename = input["path"].get<string>();
    if(fs::removeFile(filename)) {
        output["success"] = true;
        output["message"] = filename + " was deleted";
    }
    else{
        output["error"] = errors::makeErrorPayload(errors::NE_FS_FILRMER, filename);
    }
    return output;
}

json readDirectory(const json &input) {
    json output;
    output["returnValue"] = json::array();
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    string path = input["path"].get<string>();
    fs::DirReaderResult dirResult = fs::readDirectory(path);
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
            {"type", type}
        });
    }
    output["success"] = true;
    return output;
}

json copyFile(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"source", "destination"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    string source = input["source"].get<string>();
    string destination = input["destination"].get<string>();
    #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
    string command = "cp \"" + source + "\" \"" + destination + "\"";
    os::CommandResult commandResult = os::execCommand(command);
    if(commandResult.stdErr.empty()) {

    #elif defined(_WIN32)
    if(CopyFile(helpers::str2wstr(source).c_str(), helpers::str2wstr(destination).c_str(), false) == 1) {
    #endif
        output["success"] = true;
        output["message"] = "File copy operation was successful";
    }
    else{
        output["error"] = errors::makeErrorPayload(errors::NE_FS_COPYFER, source + " -> " + destination);
    }
    return output;
}

json moveFile(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"source", "destination"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    string source = input["source"].get<string>();
    string destination = input["destination"].get<string>();
    #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
    string command = "mv \"" + source + "\" \"" + destination + "\"";
    os::CommandResult commandResult = os::execCommand(command);
    if(commandResult.stdErr.empty()) {

    #elif defined(_WIN32)
    if(MoveFile(helpers::str2wstr(source).c_str(), helpers::str2wstr(destination).c_str()) == 1) {
    #endif
        output["success"] = true;
        output["message"] = "File move operation was successful";
    }
    else{
        output["error"] = errors::makeErrorPayload(errors::NE_FS_MOVEFER, source + " -> " + destination);
    }
    return output;
}

json getStats(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
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
        output["error"] = errors::makeMissingArgErrorPayload();
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
        output["error"] = errors::makeMissingArgErrorPayload();
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

} // namespace controllers

} // namespace fs
