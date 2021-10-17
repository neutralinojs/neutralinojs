#ifndef NEU_RESOURCES_H
#define NEU_RESOURCES_H

#include <string>

#include "lib/json/json.hpp"
#include "api/filesystem/filesystem.h"

using namespace std;
using json = nlohmann::json;

namespace resources {
    pair<int, string> seekFilePos(string path, json node, string curpath);
    bool makeFileTree();
    fs::FileReaderResult getFileContent(string filename);
    void extractFile(string filename, string outputFilename);
} // namespace resources

#endif // #define NEU_RESOURCES_H

