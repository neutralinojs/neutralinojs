#include <iostream>
#include <fstream>
#include "../lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;
json options;

namespace settings {

    string getFileContent(string filename){
        ifstream t(filename);
        string buffer = "";
        string line = "";
        while(!t.eof()){
            getline(t, line);
            buffer += line + "\r\n";
        }
        t.close();
        return buffer;
    }

    json getOptions(){
        return options;
    }

    json getSettings() {
        json settings;
        try {
            settings = json::parse(getFileContent("app\\settings.json"));
        }
        catch(exception e){
            cout << e.what() << endl;
        }
        options = settings;
        return options;
    }

    string getGlobalVars(){
        json settings = getOptions();
        string s = "let NL_OS='Windows';";
        s += "let NL_NAME='" + settings["appname"].get<std::string>() + "';"; 
        s += "let NL_PORT=" + settings["appport"].get<std::string>() + ";"; 
        return s;
    }

}