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

#ifndef LOG_H
#define LOG_H

#include <memory>
#include <mutex>
#include <iostream>

class log {
private:
    static std::mutex _mutex;
    std::lock_guard<std::mutex> _lock_guard;

    log() : _lock_guard (_mutex) {}

    log(const log&) = delete;
    log& operator=(const log&) = delete;
    log(log&&) : _lock_guard(_mutex) {}
    log& operator=(log&&) = default;

    

public:
    ~log() {
        std::cout << "\n";
    }

    template<typename T>
    log& operator <<(const T& val) {
        std::cout << val;
        return *this;
    }

    static log Log(const std::string& prefix, const std::string& file, const std::string& func) {
        std::cout << prefix << " [" + file + ":" + func + "] ";
        return log();
    }
};

#define INFO() log::Log("INFO",__FILE__, __func__)
#define DEBUG() log::Log("DEBUG",__FILE__, __func__)
#define TRACE() log::Log("TRACE",__FILE__, __func__)
#define ERROR() log::Log("ERROR",__FILE__, __func__)
#define WARN() log::Log("WARN",__FILE__, __func__)
#define FIXME() log::Log("FIXME",__FILE__, __func__)

#endif
