#include <string>

using namespace std;

namespace authbasic {
    void generateToken();
    bool verifyToken(string token);
    string getToken();

}
