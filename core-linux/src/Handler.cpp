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
#include "requestparser.h"

using namespace std;

void Handler::handle(int _connfd)


{

    Buffer _inputBuffer;
    Buffer _outputBuffer;

    int recvbuflen = 1024;
    char recvbuf[recvbuflen];
    int bytes_received;
    RequestParser parser;

    parser.reset();

    bool headers_ready = false;
    while(!parser.isParsingDone()) {
        bytes_received = recv(_connfd, recvbuf, recvbuflen, 0);
        if(bytes_received > 0) {
            parser.processChunk(recvbuf, bytes_received);
            if(parser.allHeadersAvailable()) {
                headers_ready = true;
            }
        } else {
            close(_connfd);
        }
    }
    pair<string, string> resp =  routes::handle(parser.getPath(), parser.getBody(), parser.getHeader("Authorization"));
    string content =  resp.first;
    std::string msg = "HTTP/1.1 200 OK\r\nContent-Type:" + resp.second + "\r\nContent-Length: " + std::to_string(content.size()) +"\r\nConnection: close\r\n\r\n" + content;
    _outputBuffer.append(msg.c_str(), msg.size());
    _outputBuffer.sendFd(_connfd);
    close(_connfd);
}
