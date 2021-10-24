#ifndef NEU_AUTHBASIC_H
#define NEU_AUTHBASIC_H

#include <string>

using namespace std;

namespace authbasic {
    void generateToken();
    bool verifyToken(const string &token);
    string getToken();
} // namespace authbasic

#endif // #define NEU_AUTHBASIC_H
