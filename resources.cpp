#include <string>
#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <limits.h>

#include "lib/easylogging/easylogging++.h"
#include "lib/json/json.hpp"
#include "helpers.h"
#include "settings.h"
#include "api/debug/debug.h"
#include "api/filesystem/filesystem.h"

#define APP_RES_FILE "/res.neu"

using namespace std;
using json = nlohmann::json;
json fileTree = nullptr;
unsigned int asarHeaderSize;

namespace resources {

    pair<int, string> seekFilePos(const string &path, json node, const string &curpath) {
        vector <string> pathSegments = helpers::split(path, '/');
        string filename = pathSegments[pathSegments.size() - 1];
        json json = node;
        for(auto pathSegment: pathSegments) {
            if(pathSegment.length() == 0 || json.is_null() || json["files"].is_null())
                continue;
            json = json["files"][pathSegment];
        }
        if(!json.is_null())
            return make_pair<int, string>(json["size"].get<int>(), json["offset"].get<string>());
        return make_pair<int, string>(-1, "");
    }

    fs::FileReaderResult getFileContent(const string &filename) {
        fs::FileReaderResult fileReaderResult;
        pair<int, string> p = seekFilePos(filename, fileTree, "");
        if(p.first != -1) {
            ifstream asarArchive;
            string resFileName = APP_RES_FILE;
            resFileName = settings::joinAppPath(resFileName);
            asarArchive.open(resFileName, ios::binary);
            if (!asarArchive) {
                debug::log("ERROR", "Resource file tree generation error: " + resFileName + " is missing.");
                fileReaderResult.hasError = true;
                return fileReaderResult;
            }
            unsigned int uSize = p.first;
            unsigned int uOffset = stoi(p.second);

            vector<char>fileBuf ( uSize );
            asarArchive.seekg(asarHeaderSize + uOffset);
            asarArchive.read(fileBuf.data(), uSize);
            string fileContent(fileBuf.begin(), fileBuf.end());
            fileReaderResult.data = fileContent;
       }
       else {
            fileReaderResult.hasError = true;
       }
       return fileReaderResult;
    }

    bool makeFileTree() {
        ifstream asarArchive;
        string resFileName = APP_RES_FILE;
        resFileName = settings::joinAppPath(resFileName);
        asarArchive.open(resFileName, ios::binary);
        if (!asarArchive) {
            debug::log("ERROR", "Resource file tree generation error: " + resFileName + " is missing.");
            return false;
        }

        char *sizeBuf = new char[8];
        asarArchive.read(sizeBuf, 8);
        unsigned int uSize = *(unsigned int *)(sizeBuf + 4) - 8;

        delete[] sizeBuf;

        asarHeaderSize = uSize + 16;
        vector<char> headerBuf(uSize);
        asarArchive.seekg(16);
        asarArchive.read(headerBuf.data(), uSize);
        json files;
        string headerContent(headerBuf.begin(), headerBuf.end());
        try {
            files = json::parse(headerContent);
        }
        catch(exception e) {
            debug::log("ERROR", e.what());
        }
        fileTree = files;
        return fileTree != nullptr;
    }
    
    void extractFile(const string &filename, const string &outputFilename) {
        fs::FileReaderResult fileReaderResult = resources::getFileContent(filename);
        fs::FileWriterOptions fileWriterOptions;
        fileWriterOptions.filename = outputFilename;
        fileWriterOptions.data = fileReaderResult.data;
        fs::writeFile(fileWriterOptions);
    }

} // namespace resources
