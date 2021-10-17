#include <string>

#include "helpers.h"

using namespace std;
string token = "";

namespace authbasic {

    void generateToken() {
        token = helpers::generateToken();
    }

    string getToken() {
        return token;
    }

    bool verifyToken(string accessToken) {
        return token == accessToken;
    }
} // namespace authbasic
