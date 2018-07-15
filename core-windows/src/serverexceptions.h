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

#ifndef SERVEREXCEPTIONS_H
#define SERVEREXCEPTIONS_H

/**
 * @file    serverexceptions.h
 * @brief   File contains definitions of exceptions being thrown by ServerListener
 */

/**
 * @brief Parent class for all exceptions being thrown by the ServerListener
 */
class ServerException : public std::runtime_error {
public:
    /**
     * @param info Information about the exception (possibly shown to the user)
     */
    ServerException(std::string info) : std::runtime_error(info) {}

    virtual ~ServerException() {}
};

/**
 * @brief Exception being thrown in case of socket library initialization error
 */
class ServerStartupException : public ServerException {
public:
    ServerStartupException()
        : ServerException("Socket library initialization failed") {}
    virtual ~ServerStartupException() {}
};


/**
 * @brief Exception being thrown in case of addrinfo() returning an error code
 */
class AddrinfoException : public ServerException {
public:
    AddrinfoException(int error_no)
        : ServerException(
              std::string("addrinfo() failed with error: ") +
              std::to_string(error_no)
        ) {}
    virtual ~AddrinfoException() {}
};


/**
 * @brief Exception being thrown in case of socket() returning an error code
 */
class SocketCreationException : public ServerException {
public:
    SocketCreationException(int error_no)
        : ServerException(
              std::string("socket() failed with error: ") +
              std::to_string(error_no)
        ) {}
    virtual ~SocketCreationException() {}
};


/**
 * @brief Exception being thrown in case of bind() returning an error code
 */
class SocketBindingException : public ServerException {
public:
    SocketBindingException(int error_no)
        : ServerException(
              std::string("bind() failed with error: ") +
              std::to_string(error_no)
        ) {}
    virtual ~SocketBindingException() {}
};


/**
 * @brief Exception being thrown in case of listen() returning an error code
 */
class ListenException : public ServerException {
public:
    ListenException(int error_no)
        : ServerException(
              std::string("listen() failed with error: ") +
              std::to_string(error_no)
        ) {}
    virtual ~ListenException() {}
};


/**
 * @brief Exception being thrown in case of accept() returning an error code
 */
class ClientAcceptationException : public ServerException {
public:
    ClientAcceptationException(int error_no)
        : ServerException(
              std::string("accept() failed with error: ") +
              std::to_string(error_no)
        ) {}
    virtual ~ClientAcceptationException() {}
};

#endif // SERVEREXCEPTIONS_H

