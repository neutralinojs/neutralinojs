#ifndef COMPUTER_H
#define COMPUTER_H

#include <map>

namespace computer {

    string getRamUsage(string json);
    
    typedef string (*pfunc)(string);

    map <string, pfunc> funcmap = {
        {"computer.getRamUsage", computer::getRamUsage}
    };
}

#endif