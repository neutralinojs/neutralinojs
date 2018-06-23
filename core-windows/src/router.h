#include <string>

using namespace std;

namespace routes {

    string getFile(string file);

    string getClientJs();

    string getIndex();

    pair<string, string> handle(string path, string j);
}