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

#include "Handler.h"
#include "router.h"

using namespace std;

void Handler::handle(int _connfd)


{

    Buffer _inputBuffer;
    Buffer _outputBuffer;
    HTTPRequest _request;

    if(_inputBuffer.readFd(_connfd) == 0){
        close(_connfd);      
        return;
    }
    std::string request = _inputBuffer.readAllAsString();
    Parser p(request);
    _request = p.getParseResult();

    pair<string, string> resp = routes::handle(_request.uri, _request.body, _request.auth);
    string content =  resp.first;
    std::string msg = "HTTP/1.1 200 OK\r\nContent-Type:" + resp.second + "\r\nContent-Length: " + std::to_string(content.size()) +"\r\nConnection: close\r\n\r\n" + content;
    _outputBuffer.append(msg.c_str(), msg.size());
    _outputBuffer.sendFd(_connfd);
    close(_connfd);
}
