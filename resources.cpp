#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <limits.h>
#include "lib/easylogging/easylogging++.h"
#include "lib/json.hpp"
#include "helpers.h"
#include "settings.h"

#define APP_RES_FILE "/res.neu"

using namespace std;
using json = nlohmann::json;
json fileTree = nullptr;
unsigned int asarHeaderSize;

namespace resources {

    pair<int, string> seekFilePos(string path, json node, string curpath) {
        vector <string> pathSegments = helpers::split(path, '/');
        string filename = pathSegments[pathSegments.size() - 1];
        json json = node;
        for(auto pathSegment: pathSegments) {
            if(pathSegment.length() == 0 || json.is_null() || json["files"].is_null())
                continue;
            json = json["files"][pathSegment];
        }
        if(!json.is_null())
            return make_pair<int, string>(json["size"].get<int>(), json["offset"].get<std::string>());
        return make_pair<int, string>(-1, "");
    }

    string getFileContent(string filename) {
        pair<int, string> p = seekFilePos(filename, fileTree, "");
        if(p.first != -1) {
            std::ifstream asarArchive;
            std::string resFileName = APP_RES_FILE;
            resFileName = settings::joinAppPath(resFileName);
            asarArchive.open(resFileName, std::ios::binary);
            if (!asarArchive) {
                LOG(ERROR) << "Resource file tree generation error: " << resFileName << " is missing.";
                return "";
            }
            unsigned int uSize = p.first;
            unsigned int uOffset = std::stoi(p.second);

            std::vector<char>fileBuf ( uSize );
            asarArchive.seekg(asarHeaderSize + uOffset);
            asarArchive.read(fileBuf.data(), uSize);
            std::string fileContent(fileBuf.begin(), fileBuf.end());
            return fileContent;
       }
       else {
           return "";
       }
    }

    bool makeFileTree() {
        std::ifstream asarArchive;
        std::string resFileName = APP_RES_FILE;
        resFileName = settings::joinAppPath(resFileName);
        asarArchive.open(resFileName, std::ios::binary);
        if (!asarArchive) {
            LOG(ERROR) << "Resource file tree generation error: " << resFileName << " is missing.";
            return false;
        }

        char *sizeBuf = new char[8];
        asarArchive.read(sizeBuf, 8);
        unsigned int uSize = *(unsigned int *)(sizeBuf + 4) - 8;

        delete[] sizeBuf;

        asarHeaderSize = uSize + 16;
        std::vector<char> headerBuf(uSize);
        asarArchive.seekg(16); // skip header
        asarArchive.read(headerBuf.data(), uSize);
        json files;
        std::string headerContent(headerBuf.begin(), headerBuf.end());
        try {
            files = json::parse(headerContent);
        }
        catch(exception e) {
            LOG(ERROR) << e.what();
        }
        fileTree = files;
        return fileTree != nullptr;
    }

}
