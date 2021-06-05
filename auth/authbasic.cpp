#include <string>
#include <vector>
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

    bool verifyToken(string resToken) {
        vector<string> tokenparts = helpers::split(resToken,  ' ');
        return token == tokenparts[1];
    }
}
