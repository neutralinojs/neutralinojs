#ifndef NEU_PERMISSION_H
#define NEU_PERMISSION_H

#include <string>

using namespace std;

namespace permission {
    void init();
    bool hasMethodAccess(const string &func);
    bool hasAPIAccess(const string &func);
} // namespace permission

#endif // #define NEU_PERMISSION_H
