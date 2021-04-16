#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include "nlohmann/json.hpp"
#include "../settings.h"

using namespace std;
using json = nlohmann::json;





namespace privileges {

    string mode = "";
    vector <string> blacklist;


    string getMode() {
        if(mode != "") {
            return mode;
        }
        else {
            json options = settings::getOptions()["mode"];
            return options;
        }

    }

    vector<string> getBlacklist() {
        if(blacklist.size() != 0 || privileges::getMode() == "browser") {
            return blacklist;
        }
        else if(privileges::getMode() == "cloud") {
            json options = settings::getOptions()["cloud"]["blacklist"];
            vector<string> s = options;
            blacklist = s;
            return blacklist;
        }
        return vector<string>();
    }


    bool checkPermission(string func) {
        for(int i = 0; i < blacklist.size(); i++) {
            if(blacklist[i] == func) return false;
        }
        return true;
    }



}
