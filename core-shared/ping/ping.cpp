#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include "settings.h"

#if defined(__linux__)
#include "../../core-linux/src/api/app/app.h"

#elif defined(_WIN32)
#include "../../core-windows/src/api/app/app.h"
#endif

using namespace std;

bool isActive = true;

void setInterval(std::function<void (void)> function,int interval) {
    thread pingThread([=]() {
        while(true) {
            std::this_thread::sleep_for(5s);
            function();
        }
    });
    pingThread.detach();
}


namespace ping {

    void receivePing() {
        isActive = true;
    }

    void pingTick() {
        if(!isActive) {
            app::exit(nullptr);
        }
        isActive = false;
    }

    void startPingReceiver() {
        if(settings::getMode() == "browser") {
            setInterval([]() {
                pingTick();
            },
            10000);
        }
    }

}
