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

#include "Parser.h"

Parser::Parser(const std::string request)
{
    assert(request.size() > 0);
    this->_request = request;
}

HTTPRequest Parser::getParseResult()
{
    parseLine();
    parseRequestLine();
    parseHeaders();
    parseBody();
    return _parseResult;
}

void Parser::parseLine()
{
    std::string::size_type lineBegin = 0;   
    std::string::size_type checkIndex = 0;  

    while(checkIndex < _request.size())
    {
        if(_request[checkIndex] == '\r')
        {
            if((checkIndex + 1) == _request.size())
            {
                std::cout << "Request not to read the complete." << std::endl;
                return;
            }
            else if(_request[checkIndex+1] == '\n')
            {
                _lines.push_back(std::string(_request, lineBegin,
                            checkIndex - lineBegin));
                checkIndex += 2;
                lineBegin = checkIndex;
            }
            else
            {
                std::cout << "Request error." << std::endl;
                return;
            }
        }
        else
            ++checkIndex;
    }
    return;
}

void Parser::parseRequestLine()
{
    assert(_lines.size() > 0);
    std::string requestLine = _lines[0];

    auto first_ws = std::find_if(requestLine.cbegin(), requestLine.cend(),
            [](char c)->bool { return (c == ' ' || c == '\t'); });

    if(first_ws == requestLine.cend())
    {
        std::cout << "Request error." << std::endl;
        return;
    }

    _parseResult.method = std::string(requestLine.cbegin(), first_ws);


    auto reverse_last_ws = std::find_if(requestLine.crbegin(), requestLine.crend(),
            [](char c)->bool { return (c == ' ' || c == '\t'); });
    auto last_ws = reverse_last_ws.base();
    _parseResult.version = std::string(last_ws, requestLine.cend());

    while((*first_ws == ' ' || *first_ws == '\t') && first_ws != requestLine.cend())
        ++first_ws;

    --last_ws;
    while((*last_ws == ' ' || *last_ws == '\t') && last_ws != requestLine.cbegin())
        --last_ws;

    _parseResult.uri = std::string(first_ws, last_ws + 1);
}

void Parser::parseHeaders()
{
    assert(_lines.size() > 0);
    for(int i = 1; i < _lines.size(); ++i)
    {
        if(_lines[i].empty()) 
            return;
        else if(strncasecmp(_lines[i].c_str(), "Host:", 5) == 0) 
        {
            auto iter = _lines[i].cbegin() + 5;
            while(*iter == ' ' || *iter == '\t')
                ++iter;
            _parseResult.host = std::string(iter, _lines[i].cend());
        }
        else if(strncasecmp(_lines[i].c_str(), "Connection:", 11) == 0) 
        {
            auto iter = _lines[i].cbegin() + 11;
            while(*iter == ' ' || *iter == '\t')
                ++iter;
            _parseResult.connection = std::string(iter, _lines[i].cend());
        }
        else if(strncasecmp(_lines[i].c_str(), "Authorization:", 14) == 0) 
        {
            auto iter = _lines[i].cbegin() + 14;
            while(*iter == ' ' || *iter == '\t')
                ++iter;
            _parseResult.auth = std::string(iter, _lines[i].cend());
        }
        else
        {
            
        }
    }
}


void Parser::parseBody() {
    bool startBody = false;
    _parseResult.body = "";
    for(int i = 0; i < _request.size(); i++){
        if(i + 3 < _request.size()) {
            if(_request[i] == '\r' && _request[i + 1] == '\n' && _request[i + 2] == '\r' && _request[i + 3] == '\n'  ){
                startBody = true;
            }
        }
        if(startBody) {
            _parseResult.body += _request[i];
        }
    }
    
}
