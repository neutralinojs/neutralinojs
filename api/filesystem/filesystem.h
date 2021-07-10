#ifndef FILESYSTEM_H
#define FILESYSTEM_H

namespace fs {
    bool createDirectory(string path);
    bool removeFile(string filename);

namespace controllers {
    json createDirectory(json input);
    json removeDirectory(json input);
    json writeFile(json input);
    json readFile(json input);
    json removeFile(json input);
    json readDirectory(json input);
} // namespace controllers
} // namespace fs

#endif
