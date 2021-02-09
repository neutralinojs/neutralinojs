// MIT License

// Copyright (c) 2018 Neutralinojs

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <unistd.h>
#include <limits.h>
#include "../lib/json/json.hpp"
#include "functions.h"
#include "log.h"

#define APP_RES_FILE "res.neu"

using namespace std;
using json = nlohmann::json;
json fileTree = nullptr;
unsigned int asarHeaderSize;

namespace resources {

    pair<int, string> seekFilePos(string path, json node, string curpath) {
        vector <string> pathSegments = functions::split(path, '/');
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

    string transformUrl(string url) {
        if(regex_search(url, regex("^app/")))
            return regex_replace(url, regex("^app/"), "");
        else if(url == "/neutralino.js")
            return "/assets/neutralino.js";
        else
            return url;
    }

    string getFileContent(string filename) {
        pair<int, string> p = seekFilePos(transformUrl(filename), fileTree, "");
        if(p.first != -1) {
            std::ifstream asarArchive;
            asarArchive.open(APP_RES_FILE, std::ios::binary);
            if (!asarArchive) {
                ERROR() << "resources::makeFileTree error: " << APP_RES_FILE << " is missing.";
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

    void makeFileTree() {
        std::ifstream asarArchive;
        asarArchive.open(APP_RES_FILE, std::ios::binary);
        if (!asarArchive) {
            ERROR() << "resources::makeFileTree error: " << APP_RES_FILE << " is missing.";
            return;
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
            ERROR() << "resources::makeFileTree error: " << strerror(errno);
        }
        fileTree = files;
    }

}
