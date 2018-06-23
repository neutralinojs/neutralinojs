#include <iostream>
#include "serverlistener.h"

#define APP_NAME "Neutralino"
#define APP_VERSION "1.0.0"

using namespace std;

int main() {
    cout << APP_NAME << " " << APP_VERSION << "" << endl;
    ServerListener().run();
    
    return 0;
}

