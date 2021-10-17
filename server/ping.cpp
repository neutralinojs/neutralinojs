#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

#include "api/app/app.h"

using namespace std;

bool isActive = true;

namespace ping {

    void __setInterval(std::function<void (void)> function) {
        thread pingThread([=]() {
            while(true) {
                std::this_thread::sleep_for(5s);
                function();
            }
        });
        pingThread.detach();
    }

    void pingReceived() {
        isActive = true;
    }

    void pingTick() {
        if(!isActive) {
            app::exit();
        }
        isActive = false;
    }

    void start() {
        __setInterval([]() {
            pingTick();
        });
    }

} // namespace ping
