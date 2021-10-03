#include <string>

#include "lib/json.hpp"
#include "api/filesystem/filesystem.h"

using namespace std;
using json = nlohmann::json;

namespace resources {
    pair<int, string> seekFilePos(string path);
    bool makeFileTree();
    fs::FileReaderResult getFileContent(string filename);
    void extractFile(string filename, string outputFilename);
}
