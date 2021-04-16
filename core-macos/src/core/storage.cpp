#include <iostream>
#include <fstream>
#include "include/storage.h"
#include "nlohmann/json.hpp"
#include "../settings.h"

using namespace std;
using json = nlohmann::json;

namespace storage {
    string getData(string jso) {
        json input;
        json output;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }
        string filename = "storage/" + input["bucket"].get<std::string>() + ".json";
        output["content"] = settings::getFileContent(filename);
        return output.dump();
    }

    string putData(string jso) {
        json input;
        json output;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }

        string filename = "storage/" + input["bucket"].get<std::string>() + ".json";
        string content = input["content"].dump();
        ofstream t(filename);
        t << content;
        t.close();

        return output.dump();
    }

}
