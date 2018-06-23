#include <iostream>
#include <sstream>
#include <vector>

using namespace std;
namespace functions {

    vector<string> split(const string &s, char delim) {
        stringstream ss(s);
        string item;
        vector<string> tokens;
        while (getline(ss, item, delim)) {
            tokens.push_back(item);
        }
        return tokens;
    }

}