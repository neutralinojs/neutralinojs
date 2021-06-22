#include <vector>
#include <string>

using namespace std;

namespace helpers {
    vector<string> split(const string &s, char delim);
    string generateToken();
    void urldecode(char *dst, const char *src);
    char* cStrCopy(string str);
}
