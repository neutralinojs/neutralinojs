#include <string>
#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <filesystem>
#include <limits.h>

#include "lib/postject/postject-api.h"
#include "lib/easylogging/easylogging++.h"
#include "lib/json/json.hpp"
#include "helpers.h"
#include "errors.h"
#include "settings.h"
#include "resources.h"
#include "api/debug/debug.h"
#include "api/fs/fs.h"

#define NEU_APP_RES_FILE "/resources.neu"
#define RESOURCE_NAME_EMBEDDED "NEUTRALINOJS_RESOURCES_NEU"

using namespace std;
using json = nlohmann::json;

namespace resources {

json fileTree = nullptr;
unsigned int asarHeaderSize;
resources::ResourceMode mode = resources::ResourceModeEmbedded;

pair<unsigned long, string> __seekFilePos(const string &path, json node) {
    vector<string> pathSegments = helpers::split(path, '/');
    json json = node;
    for(const auto &pathSegment: pathSegments) {
        if(pathSegment.size() == 0 || json.is_null() || json["files"].is_null())
            continue;
        json = json["files"][pathSegment];
    }
    if(!json.is_null())
        return make_pair(json["size"].get<unsigned long>(), json["offset"].get<string>());
    return make_pair(-1, "");
}

// Needs explicit close later
ifstream __openResourceFile() {
    ifstream asarArchive;
    string resFileName = NEU_APP_RES_FILE;
    resFileName = settings::joinAppPath(resFileName);
    asarArchive.open(CONVSTR(resFileName), ios::binary);
    if(!asarArchive) {
        debug::log(debug::LogTypeError, errors::makeErrorMsg(errors::NE_RS_TREEGER, resFileName));
    }
    return asarArchive;
}

fs::FileReaderResult __getFileFromBundle(const string &filename) {
    fs::FileReaderResult fileReaderResult;
    pair<long, string> p = __seekFilePos(filename, fileTree);
    if(p.first != -1) {
        ifstream asarArchive = __openResourceFile();
        if (!asarArchive) {
            fileReaderResult.status = errors::NE_RS_TREEGER;
            return fileReaderResult;
        }
        unsigned long size = p.first;
        unsigned long uOffset = stoi(p.second);

        vector<char>fileBuf ( size );
        asarArchive.seekg(asarHeaderSize + uOffset);
        asarArchive.read(fileBuf.data(), size);
        string fileContent(fileBuf.begin(), fileBuf.end());
        fileReaderResult.data = fileContent;
        asarArchive.close();
   }
   else {
        fileReaderResult.status = errors::NE_RS_NOPATHE;
   }
   return fileReaderResult;
}

fs::FileReaderResult __getFileFromEmbedded(const string &filename) {
    fs::FileReaderResult fileReaderResult;
    pair<long, string> p = __seekFilePos(filename, fileTree);
    if(p.first != -1) {
        size_t resource_size = 0;
        const void* resource_ptr = postject_find_resource(RESOURCE_NAME_EMBEDDED, &resource_size, NULL);
        if (resource_ptr == NULL || resource_size <= 0) {
            fileReaderResult.status = errors::NE_RS_TREEGER;
            return fileReaderResult;
        }

        const unsigned char* bytes = (const unsigned char*)resource_ptr;
        unsigned long size = p.first;
        unsigned long uOffset = stoi(p.second);

        vector<char>fileBuf ( size );
        for (size_t i = asarHeaderSize + uOffset; i < asarHeaderSize + uOffset + size; i++) {
          fileBuf[i - (asarHeaderSize + uOffset)] = bytes[i];
        }

        string fileContent(fileBuf.begin(), fileBuf.end());
        fileReaderResult.data = fileContent;
   }
   else {
        fileReaderResult.status = errors::NE_RS_NOPATHE;
   }
   return fileReaderResult;
}

bool __makeBundleFileTree() {
    ifstream asarArchive = __openResourceFile();
    if (!asarArchive) {
        return false;
    }

    char *sizeBuf = new char[8];
    asarArchive.read(sizeBuf, 8);
    unsigned int size = *(unsigned int *)(sizeBuf + 4) - 8;

    delete[] sizeBuf;

    asarHeaderSize = size + 16;
    vector<char> headerBuf(size);
    asarArchive.seekg(16);
    asarArchive.read(headerBuf.data(), size);
    json files;
    string headerContent(headerBuf.begin(), headerBuf.end());
    asarArchive.close();
    try {
        files = json::parse(headerContent);
    }
    catch(exception e) {
        debug::log(debug::LogTypeError, e.what());
    }
    fileTree = files;
    return fileTree != nullptr;
}

bool __makeEmbeddedFileTree() {
    size_t resource_size = 0;
    const void* resource_ptr = postject_find_resource(RESOURCE_NAME_EMBEDDED, &resource_size, NULL);
    if (resource_ptr == NULL || resource_size <= 0) {
      return false;
    }

    const unsigned char* bytes = (const unsigned char*)resource_ptr;
    char *sizeBuf = new char[8];
    for (size_t i = 0; i < 8; ++i) {
      sizeBuf[i] = bytes[i];
    }

    unsigned int size = *(unsigned int *)(sizeBuf + 4) - 8;
    delete[] sizeBuf;

    asarHeaderSize = size + 16;
    vector<char> headerBuf(size);
    for (size_t i = 16; i < 16 + size; ++i) {
      headerBuf[i - 16] = bytes[i];
    }

    json files;
    string headerContent(headerBuf.begin(), headerBuf.end());
    try {
        files = json::parse(headerContent);
    }
    catch(exception e) {
        debug::log(debug::LogTypeError, e.what());
    }
    fileTree = files;
    return fileTree != nullptr;
}

bool extractFile(const string &filename, const string &outputFilename) {
    fs::FileReaderResult fileReaderResult = resources::getFile(filename);
    if(fileReaderResult.status != errors::NE_ST_OK) {
        return false;
    }
    auto extractPath = filesystem::path(CONVSTR(outputFilename));
    if(!extractPath.parent_path().empty()) {
        filesystem::create_directories(extractPath.parent_path());
    }
   
    fs::FileWriterOptions fileWriterOptions;
    fileWriterOptions.filename = outputFilename;
    fileWriterOptions.data = fileReaderResult.data;
    return fs::writeFile(fileWriterOptions);
}

fs::FileReaderResult getFile(const string &filename) {
    if (resources::isEmbeddedMode()) {
        return __getFileFromEmbedded(filename);
    }
    if(resources::isBundleMode()) {
        return __getFileFromBundle(filename);
    }
    return fs::readFile(settings::joinAppPath(filename));
}

void init() {
    if(resources::isDirMode()) {
        return;
    }
    if (resources::isEmbeddedMode()) {
        if (postject_has_resource() && __makeEmbeddedFileTree()) {
            return;
        }
        resources::setMode(resources::ResourceModeBundle); // Try bundle mode
    }
    bool resourceLoaderStatus = __makeBundleFileTree();
    if(!resourceLoaderStatus) {
        resources::setMode(resources::ResourceModeDir); // fallback to directory mode
    }
}

void setMode(const resources::ResourceMode m) {
    mode = m;
}

resources::ResourceMode getMode() {
    return mode;
}

bool isDirMode() {
   return resources::getMode() == resources::ResourceModeDir;
}

bool isBundleMode() {
   return resources::getMode() == resources::ResourceModeBundle;
}

bool isEmbeddedMode() {
   return resources::getMode() == resources::ResourceModeEmbedded;
}

json getFileTree() {
   return fileTree;
}

string getModeString() {
    if (resources::isDirMode()) return "directory";
    else if (resources::isBundleMode()) return "bundle";
    else return "embedded";
}

} // namespace resources
