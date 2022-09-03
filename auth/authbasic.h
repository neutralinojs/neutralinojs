#ifndef NEU_AUTHBASIC_H
#define NEU_AUTHBASIC_H

#include <string>

using namespace std;

namespace authbasic {

enum TokenSecurity { TokenSecurityOneTime, TokenSecurityNone };

void init();
void exportAuthInfo();
bool verifyToken(const string &token);
string getToken();
string getTokenInternal();

} // namespace authbasic

#endif // #define NEU_AUTHBASIC_H
