#ifndef OS_H
#define OS_H

#include <map>

namespace os {

    string runCommand(string jso);
    string getEnvar(string jso);
    
    typedef string (*pfunc)(string);

    map <string, pfunc> funcmap = {
        {"os.runCommand", os::runCommand },
        {"os.getEnvar", os::getEnvar}
    };
}

#endif