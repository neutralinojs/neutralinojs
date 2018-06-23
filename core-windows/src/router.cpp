#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "functions.h"
#include "core/filesystem.h"
#include "core/os.h"
#include "core/computer.h"
#include "settings.h"
#include "../lib/json/json.hpp"


using namespace std;
using namespace filesystem;
using namespace os;
using json = nlohmann::json;

namespace routes {
    
    string getFile(string file) {
        ifstream t;
        t.open(file);
        string buffer = "";
        string line = "";
        while(!t.eof()){
            getline(t, line);
            buffer += line + "\r\n";
        }
        t.close();
        return buffer;
    }

    string getClientJs() {
        return routes::getFile("app\\assets\\neutralino.js");
    }    

    string getIcon() {
        return "data:image/png;base64,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";
    }

    string getIndex() {
        return "<html style=\"width: 100%; height: 100%; position: absolute; background-repeat:no-repeat; background-position: center; background-color: black; background-image:url('" + routes::getIcon() + "')\"></html>";
    }

    pair<string, string> handle(string path, string j) {
        json options = settings::getOptions();
        string appname = options["appname"];
        if(path == "/" +  appname ){
            return make_pair(routes::getFile("app\\index.html"), "text/html");
        }
        else if(path == "/neutralino.js"){
            return make_pair(routes::getClientJs() + settings::getGlobalVars(), "text/javascript");
        }
        else if(path == "/settings.json"){
            return make_pair(settings::getSettings().dump(), "application/json");
        }
        else if(path.find("/assets") != string::npos && path.find(".js") != string::npos){
            return make_pair(routes::getFile("app" + path), "text/javascript");
        }
        else if(path.find("/assets") != string::npos && path.find(".css") != string::npos){
            return make_pair(routes::getFile("app" + path), "text/css");
        }
        else if(path == "/") {
            return make_pair(routes::getIndex(), "text/html");
        }
        else {
            vector<string> portions = functions::split(path, '/');
            if(portions.size() == 3) {
                string module = portions[1];
                string func = portions[2];
                string output = "";
                //cout << module << "."<< func << endl;
                if(filesystem::funcmap.find(module + "." + func) != filesystem::funcmap.end() ){
                    pfunc f = filesystem::funcmap[module + "." + func];
                    output = (*f)(j); 
                }
                else if(os::funcmap.find(module + "." + func) != os::funcmap.end() ){
                    pfunc f = os::funcmap[module + "." + func];
                    output = (*f)(j); 
                }
                else if(computer::funcmap.find(module + "." + func) != computer::funcmap.end() ){
                    pfunc f = computer::funcmap[module + "." + func];
                    output = (*f)(j); 
                }
                else {
                    json o = {{"erorr", module + "." + func + " is not supported"}};
                    output = o.dump();
                }

                return make_pair(output, "application/json");
            }
        }
        return make_pair(path, "application/json");
    }


}