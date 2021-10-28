#ifndef NEU_RESOURCES_H
#define NEU_RESOURCES_H

#include <string>

#include "lib/json/json.hpp"
#include "api/filesystem/filesystem.h"

using namespace std;
using json = nlohmann::json;

namespace resources {
    pair<int, string> seekFilePos(const string &path, json node, const string &curpath);
    bool makeFileTree();
    fs::FileReaderResult getFileContent(const string &filename);
    void extractFile(const string &filename, const string &outputFilename);
} // namespace resources

#endif // #define NEU_RESOURCES_H

