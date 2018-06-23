#include <map>
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

namespace filesystem {
    string createDirectory(string jso);

    string removeDirectory(string jso);

    string writeFile(string jso);

    string readFile(string jso);

    string removeFile(string jso);

    typedef string (*pfunc)(string);

    map <string, pfunc> funcmap = {
        {"filesystem.createDirectory", filesystem::createDirectory},
        {"filesystem.removeDirectory", filesystem::removeDirectory},
        {"filesystem.readFile", filesystem::readFile},
        {"filesystem.writeFile", filesystem::writeFile},
        {"filesystem.removeFile", filesystem::removeFile}
    };



}

#endif