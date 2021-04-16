#include <string>
#include <vector>
#include "../functions.h"

using namespace std;
string token = "";
namespace authbasic {

    void generateToken() {
        token = functions::generateToken();
    }

    string getToken() {
        return token;
    }

    bool verifyToken(string resToken) {
        vector<string> tokenparts = functions::split(resToken,  ' ');
        return token == tokenparts[1];
    }
}
