#include <iostream>
#include <signal.h>
#include <unistd.h>
#include "lib/json.hpp"
#include "ping/ping.h"
#include "settings.h"

using namespace std;
using json = nlohmann::json;

namespace app {
    
    string exit(json input) {
        kill(getpid(),SIGINT);
        return "";
    }

    string keepAlive(json input) {
        json output;
        ping::receivePing();
        output["message"] = "Keep alive call was successful. Server will not be terminated automatically.";
        output["success"] = true;
        return output.dump();
    }

    string getConfig(json input) {
        json output;
        output["config"] = settings::getConfig();
        output["success"] = true;
        return output.dump();
    }

    string open(json input) {
        json output;
        string url = input["url"];
        system(("open \"" + url + "\"").c_str());
        output["success"] = true;
        return output.dump();
    }

}
