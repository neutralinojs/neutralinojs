#ifndef NEU_RESOURCES_H
#define NEU_RESOURCES_H

#include <string>

#include "lib/json/json.hpp"
#include "api/fs/fs.h"

using namespace std;
using json = nlohmann::json;

namespace resources {

enum ResourceMode { ResourceModeDir, ResourceModeBundle, ResourceModeEmbedded };

fs::FileReaderResult getFile(const string &filename);
bool extractFile(const string &filename, const string &outputFilename);
void init();
void setMode(const resources::ResourceMode mode);
resources::ResourceMode getMode();
bool isDirMode();
bool isBundleMode();
bool isEmbeddedMode();
string getModeString();
json getFileTree();

} // namespace resources

#endif // #define NEU_RESOURCES_H

