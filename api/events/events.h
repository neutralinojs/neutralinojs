#ifndef EVT_H
#define EVT_H

#include <string>

using namespace std;

namespace events {
    void dispatch(string event, string data);
} // namespace events
#endif
