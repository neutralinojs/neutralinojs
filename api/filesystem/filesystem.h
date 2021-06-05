#ifndef FILESYSTEM_H
#define FILESYSTEM_H

namespace fs {
    string createDirectory(json input);
    string removeDirectory(json input);
    string writeFile(json input);
    string readFile(json input);
    string removeFile(json input);
    string readDirectory(json input);
}

#endif
