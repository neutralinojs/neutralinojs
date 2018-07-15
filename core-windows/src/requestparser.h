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

#ifndef REQUESTPARSER_H
#define REQUESTPARSER_H

/**
 * @file    requestparser.h
 * @brief   File contains definition of RequestParser
 */

#include <string>
#include <map>

/**
 * @brief Extracts data from HTTP headers, given the data received from client socket
 * @todo Implement extraction of request body (e.g. in POST requests)
 */
class RequestParser
{
    bool half_end_of_line;
    bool end_of_line;
    bool first_line;
    bool beginning;

    std::string method;
    std::string path;
    std::string proto_ver;

    std::string tmp_header_name;
    std::string tmp_header_value;

    char previous_char;

    std::map<std::string, std::string> headers;
    bool headers_available;

    std::string body;
    char prev_char;
    bool beginbody;
    int charcount;
    bool parsingDone;

public:

    /**
     * Initialize a new parser instance.
     */
    RequestParser();

    /**
     * Feeds the parser with next portion of data from client.
     * Should be called in loop, after each chunk of data is received.
     *
     * @param buf Pointer to the buffer in which data from client socket is stored
     * @param size Size of the chunk of data
     */
    void processChunk(const char *buf, size_t size);

    /**
     * Check if the end of headers was reached by the parser.
     *
     * @return Information if all data was already extracted from headers and can be safely accessed
     */
    bool allHeadersAvailable();

    /**
     * Get extracted headers.
     *
     * @return Headers in form of std::map (name -> value)
     */
    std::map<std::string, std::string> getHeaders();

    /**
     * @return String representing the HTTP method used in request
     */
    std::string getMethod();

    /**
     * @return String representing the requested path
     */
    std::string getPath();

    /**
     * @return String representing the protocol (and version) used to perform the request
     */
    std::string getProtocol();

    /**
     * Prepare the object to handle another request (clear all extracted data).
     */
    void reset();

    std::string getBody();

    bool isParsingDone();

    std::string getHeader(std::string key);

    virtual ~RequestParser() = default;
};

#endif // REQUESTPARSER_H
