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

#include <map>
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

namespace filesystem {
    string createDirectory(string jso);
    
    string removeDirectory(string jso);

    string writeFile(string jso);

    string readFile(string jso);

    string removeFile(string jso);

    string readDirectory(string jso);

    typedef string (*pfunc)(string);

    map <string, pfunc> funcmap = {
        {"filesystem.createDirectory", filesystem::createDirectory},
        {"filesystem.removeDirectory", filesystem::removeDirectory},
        {"filesystem.readFile", filesystem::readFile},
        {"filesystem.writeFile", filesystem::writeFile},
        {"filesystem.removeFile", filesystem::removeFile},
        {"filesystem.readDirectory", filesystem::readDirectory}
    };



}

#endif
