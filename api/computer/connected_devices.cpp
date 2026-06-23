#include "api/computer/connected_devices.h"

#include <codecvt>
#include <iomanip>
#include <locale>
#include <set>
#include <sstream>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "hidapi/hidapi.h"

#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#if defined(_WIN32)
#define MA_ENABLE_WASAPI
#elif defined(__APPLE__)
#define MA_ENABLE_COREAUDIO
#elif defined(__linux__)
#define MA_ENABLE_ALSA
#endif
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio/miniaudio.h"

using namespace std;

namespace computer {

namespace {

string __formatHexId(unsigned int value) {
    if(value == 0) return "";

    stringstream ss;
    ss << hex << nouppercase << setw(4) << setfill('0') << value;
    return ss.str();
}

string __wideToUtf8(const wchar_t *value) {
    if(!value) return "";

    #if defined(_WIN32)
    int size = WideCharToMultiByte(CP_UTF8, 0, value, -1, nullptr, 0, nullptr, nullptr);
    if(size <= 1) return "";

    string output(size - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, value, -1, output.data(), size, nullptr, nullptr);
    return output;
    #else
    try {
        wstring_convert<codecvt_utf8<wchar_t>> converter;
        return converter.to_bytes(value);
    }
    catch(...) {
        return "";
    }
    #endif
}

void __addDevice(vector<ConnectedDevice> &devices, set<string> &deviceKeys, ConnectedDevice device) {
    if(device.name.empty()) {
        device.name = "Unknown device";
    }

    string key = device.type + "|" + device.vendorId + "|" + device.productId + "|" + device.name;
    if(deviceKeys.find(key) != deviceKeys.end()) return;

    deviceKeys.insert(key);
    devices.push_back(device);
}

string __getHIDDeviceType(unsigned short usagePage, unsigned short usage) {
    if(usagePage == 0x01) {
        if(usage == 0x02) return "MOUSE";
        if(usage == 0x06 || usage == 0x07) return "KEYBOARD";
    }
    return "HID";
}

void __addHIDDevices(vector<ConnectedDevice> &devices, set<string> &deviceKeys) {
    if(hid_init() != 0) return;

    hid_device_info *hidDevices = hid_enumerate(0, 0);
    for(hid_device_info *hidDevice = hidDevices; hidDevice; hidDevice = hidDevice->next) {
        ConnectedDevice device;
        device.name = __wideToUtf8(hidDevice->product_string);
        device.type = __getHIDDeviceType(hidDevice->usage_page, hidDevice->usage);
        device.vendorId = __formatHexId(hidDevice->vendor_id);
        device.productId = __formatHexId(hidDevice->product_id);

        __addDevice(devices, deviceKeys, device);
    }

    hid_free_enumeration(hidDevices);
    hid_exit();
}

void __addAudioDeviceInfo(
    vector<ConnectedDevice> &devices,
    set<string> &deviceKeys,
    const ma_device_info *audioDevices,
    ma_uint32 deviceCount
) {
    for(ma_uint32 i = 0; i < deviceCount; i++) {
        ConnectedDevice device;
        device.name = audioDevices[i].name;
        device.type = "AUDIO";
        device.vendorId = "";
        device.productId = "";

        __addDevice(devices, deviceKeys, device);
    }
}

void __addAudioDevices(vector<ConnectedDevice> &devices, set<string> &deviceKeys) {
    ma_context context;
    if(ma_context_init(nullptr, 0, nullptr, &context) != MA_SUCCESS) return;

    ma_device_info *playbackDevices = nullptr;
    ma_uint32 playbackDeviceCount = 0;
    ma_device_info *captureDevices = nullptr;
    ma_uint32 captureDeviceCount = 0;

    if(ma_context_get_devices(
        &context,
        &playbackDevices,
        &playbackDeviceCount,
        &captureDevices,
        &captureDeviceCount
    ) == MA_SUCCESS) {
        __addAudioDeviceInfo(devices, deviceKeys, playbackDevices, playbackDeviceCount);
        __addAudioDeviceInfo(devices, deviceKeys, captureDevices, captureDeviceCount);
    }

    ma_context_uninit(&context);
}

} // namespace

vector<ConnectedDevice> getConnectedDevices() {
    vector<ConnectedDevice> devices;
    set<string> deviceKeys;

    __addHIDDevices(devices, deviceKeys);
    __addAudioDevices(devices, deviceKeys);

    return devices;
}

} // namespace computer
