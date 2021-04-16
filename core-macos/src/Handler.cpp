 /*
 * Author: Broglie
 * E-mail: yibo141@outlook.com
 */
 
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
