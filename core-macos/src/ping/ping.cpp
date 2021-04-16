#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include "../cloud/privileges.h"


using namespace std;

bool isActive = true;
bool firstPing = false;

template<typename Function>
void setInterval(Function function,int interval) {
    thread th([&]() {
        while(true) {
            std::this_thread::sleep_for(5s);
            function();
        }
    });
    th.detach();
}


namespace ping {

    void receivePing() {
        isActive = true;
        if(!firstPing) {
            firstPing = true;
        }
    }

    void pingTick() {
        if(!isActive && firstPing) {
            std::exit(0);
        }
        isActive = false;
    }

    void startPingReceiver() {
        if(privileges::getMode() == "browser") {
            setInterval([]() {
                pingTick();
            },
            10000);
        }
    }

}
