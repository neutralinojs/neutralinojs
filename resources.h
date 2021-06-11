#include "lib/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace resources {
    pair<int, string> seekFilePos(string path);
    bool makeFileTree();
    string getFileContent(string filename);
    string getFileContentBinary(string filename);
}
