#ifndef NEU_CONNECTED_DEVICES_H
#define NEU_CONNECTED_DEVICES_H

#include <string>
#include <vector>

using namespace std;

namespace computer {

struct ConnectedDevice {
    string name;
    string type;
    string vendorId;
    string productId;
};

vector<ConnectedDevice> getConnectedDevices();

} // namespace computer

#endif // #define NEU_CONNECTED_DEVICES_H
