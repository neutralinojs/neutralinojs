 /*
 * Author: Broglie
 * E-mail: yibo141@outlook.com
 */

#ifndef HANDLER_H
#define HANDLER_H

#include <string>
#include <algorithm>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "Buffer.h"

class Handler
{
public:
    static void handle(int _connfd);
};

#endif // HANDLER_H
