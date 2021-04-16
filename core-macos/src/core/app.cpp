#include <iostream>
#include <signal.h>
#include <unistd.h>
#include "nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace app {

    string exit(string jso) {
        kill(getpid(),SIGINT); // Experimental: TODO: check further
        return "";
    }

}
