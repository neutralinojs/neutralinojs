#include <string>

using namespace std;
string token = "";
namespace authbasic {
    
    void generateToken() {
        token = "4";
    }

    bool verifyToken(string resToken) {
        return token == resToken;
    }
}