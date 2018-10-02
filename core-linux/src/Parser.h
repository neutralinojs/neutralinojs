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

#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <string.h>


typedef struct
{
    std::string method;    
    std::string uri;        
    std::string version;   
    std::string host;   
    std::string connection; 
    std::string auth;
    std::string body;
} HTTPRequest;

class Parser
{
public:
    Parser(const std::string request);
    HTTPRequest getParseResult();
private:
    void parseLine();       
    void parseRequestLine(); 
    void parseHeaders();     
    void parseBody(); 
    std::string _request;    
    std::vector<std::string> _lines;
    HTTPRequest _parseResult;
};

#endif // PARSER_H