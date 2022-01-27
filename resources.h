#ifndef NEU_RESOURCES_H
#define NEU_RESOURCES_H

#include <string>

#include "lib/json/json.hpp"
#include "api/filesystem/filesystem.h"

using namespace std;
using json = nlohmann::json;

namespace resources {
    fs::FileReaderResult getFile(const string &filename);
    void extractFile(const string &filename, const string &outputFilename);
    void init();
    void setMode(const string &mode);
    string getMode();
} // namespace resources

#endif // #define NEU_RESOURCES_H

