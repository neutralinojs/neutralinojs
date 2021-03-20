// MIT License

// Copyright (c) 2018 Neutralinojs

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
bool firstPingReceived = false;

void setInterval(auto function,int interval) {
    thread pingThread([&]() {
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
        if(!firstPingReceived) {
            firstPingReceived = true;
        }
    }

    void pingTick() {
        if(!isActive && firstPingReceived) {
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
