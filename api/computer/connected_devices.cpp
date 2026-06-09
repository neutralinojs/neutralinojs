#include "api/computer/connected_devices.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <map>
#include <regex>
#include <set>
#include <sstream>

#if defined(_WIN32)
#define _WINSOCKAPI_
#include <windows.h>
#include <propkeydef.h>
#include <functiondiscoverykeys_devpkey.h>
#include <mmdeviceapi.h>
#include <propidl.h>
#include "helpers.h"
#elif defined(__APPLE__)
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDUsageTables.h>
#endif

using namespace std;

namespace computer {

namespace {

string __trim(const string &value) {
    size_t begin = value.find_first_not_of(" \t\r\n");
    if(begin == string::npos) return "";

    size_t end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

string __toLower(string value) {
    transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(tolower(ch));
    });
    return value;
}

string __stripHexPrefix(string value) {
    value = __trim(value);
    if(value.rfind("0x", 0) == 0 || value.rfind("0X", 0) == 0) {
        value = value.substr(2);
    }
    return __toLower(value);
}

string __formatHexId(unsigned int value) {
    if(value == 0) return "";

    stringstream ss;
    ss << hex << nouppercase << setw(4) << setfill('0') << value;
    return ss.str();
}

string __formatHexId(const string &value) {
    string hexId = __stripHexPrefix(value);
    if(hexId.empty()) return "";

    try {
        unsigned int numericValue = stoul(hexId, nullptr, 16);
        return __formatHexId(numericValue);
    }
    catch(...) {
        return hexId;
    }
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

#if defined(_WIN32)

struct __WindowsDeviceCandidate {
    ConnectedDevice device;
    string groupKey;
    int priority;
};

string __getRawInputDeviceName(HANDLE deviceHandle) {
    UINT nameSize = 0;
    if(GetRawInputDeviceInfoW(deviceHandle, RIDI_DEVICENAME, nullptr, &nameSize) != 0 || nameSize == 0) {
        return "";
    }

    wstring name(nameSize, L'\0');
    UINT result = GetRawInputDeviceInfoW(deviceHandle, RIDI_DEVICENAME, name.data(), &nameSize);
    if(result == static_cast<UINT>(-1)) return "";

    if(!name.empty() && name.back() == L'\0') {
        name.pop_back();
    }

    return helpers::wstr2str(name);
}

string __getWindowsRawInputType(DWORD type) {
    switch(type) {
        case RIM_TYPEMOUSE:
            return "MOUSE";
        case RIM_TYPEKEYBOARD:
            return "KEYBOARD";
        case RIM_TYPEHID:
            return "HID";
        default:
            return "HID";
    }
}

int __getWindowsInputPriority(const string &type) {
    if(type == "MOUSE" || type == "KEYBOARD") return 3;
    return 1;
}

bool __isWindowsSyntheticInputDevice(const string &deviceName) {
    string loweredName = __toLower(deviceName);
    return loweredName.find("converteddevice") != string::npos;
}

bool __isWindowsExternalHIDPath(const string &deviceName) {
    string loweredName = __toLower(deviceName);
    return loweredName.find("vid_") != string::npos ||
        loweredName.find("pid_") != string::npos ||
        loweredName.find("bth") != string::npos ||
        loweredName.find("bluetooth") != string::npos;
}

bool __shouldKeepWindowsInputDevice(const ConnectedDevice &device, const string &deviceName) {
    if(device.type == "MOUSE" || device.type == "KEYBOARD") return true;
    return __isWindowsExternalHIDPath(deviceName);
}

void __setWindowsIdsFromDevicePath(const string &deviceName, ConnectedDevice &device) {
    regex vendorRegex("(?:vid|VID)_([0-9a-fA-F]{4})");
    regex productRegex("(?:pid|PID)_([0-9a-fA-F]{4})");
    smatch match;

    if(regex_search(deviceName, match, vendorRegex) && match.size() > 1) {
        device.vendorId = __toLower(match[1].str());
    }

    if(regex_search(deviceName, match, productRegex) && match.size() > 1) {
        device.productId = __toLower(match[1].str());
    }
}

string __getWindowsInputGroupKey(const string &deviceName, const string &deviceType) {
    string loweredName = __toLower(deviceName);
    smatch match;

    regex vendorProductRegex("(vid_[0-9a-f]{4}&pid_[0-9a-f]{4})");
    if(regex_search(loweredName, match, vendorProductRegex) && match.size() > 1) {
        return match[1].str() + "|" + deviceType;
    }

    regex hidIdRegex("\\\\[?]\\\\hid#([^#]+)#");
    if(regex_search(loweredName, match, hidIdRegex) && match.size() > 1) {
        string hidId = regex_replace(match[1].str(), regex("&col[0-9a-f]+"), "");
        return "hid|" + hidId;
    }

    string groupKey = loweredName;
    groupKey = regex_replace(groupKey, regex("#\\{[0-9a-f-]+\\}$"), "");
    groupKey = regex_replace(groupKey, regex("&col[0-9a-f]+"), "");
    return groupKey;
}

string __getWindowsFallbackInputName(const ConnectedDevice &device) {
    if(device.type == "MOUSE") return "Mouse";
    if(device.type == "KEYBOARD") return "Keyboard";
    return "HID device";
}

void __addWindowsInputDevices(vector<ConnectedDevice> &devices, set<string> &deviceKeys) {
    UINT deviceCount = 0;
    if(GetRawInputDeviceList(nullptr, &deviceCount, sizeof(RAWINPUTDEVICELIST)) != 0 || deviceCount == 0) {
        return;
    }

    vector<RAWINPUTDEVICELIST> rawDevices(deviceCount);
    UINT result = GetRawInputDeviceList(rawDevices.data(), &deviceCount, sizeof(RAWINPUTDEVICELIST));
    if(result == static_cast<UINT>(-1)) return;

    map<string, __WindowsDeviceCandidate> selectedCandidates;
    for(UINT i = 0; i < result; i++) {
        RID_DEVICE_INFO info {};
        info.cbSize = sizeof(RID_DEVICE_INFO);
        UINT infoSize = sizeof(RID_DEVICE_INFO);
        if(GetRawInputDeviceInfoW(rawDevices[i].hDevice, RIDI_DEVICEINFO, &info, &infoSize) == static_cast<UINT>(-1)) {
            continue;
        }

        string deviceName = __getRawInputDeviceName(rawDevices[i].hDevice);
        if(__isWindowsSyntheticInputDevice(deviceName)) continue;

        ConnectedDevice device;
        device.type = __getWindowsRawInputType(info.dwType);
        device.name = deviceName;

        if(info.dwType == RIM_TYPEHID) {
            device.vendorId = __formatHexId(info.hid.dwVendorId);
            device.productId = __formatHexId(info.hid.dwProductId);
        }
        __setWindowsIdsFromDevicePath(deviceName, device);

        if(!__shouldKeepWindowsInputDevice(device, deviceName)) continue;

        if(device.name.empty()) {
            device.name = __getWindowsFallbackInputName(device);
        }

        string groupKey = __getWindowsInputGroupKey(deviceName, device.type);
        if(groupKey.empty()) {
            groupKey = device.type + "|" + device.vendorId + "|" + device.productId + "|" + device.name;
        }

        __WindowsDeviceCandidate candidate {
            device,
            groupKey,
            __getWindowsInputPriority(device.type)
        };

        auto existingCandidate = selectedCandidates.find(groupKey);
        if(existingCandidate == selectedCandidates.end() ||
            candidate.priority > existingCandidate->second.priority) {
            selectedCandidates[groupKey] = candidate;
        }
    }

    for(const auto &[groupKey, candidate]: selectedCandidates) {
        __addDevice(devices, deviceKeys, candidate.device);
    }
}

void __addWindowsAudioDevices(vector<ConnectedDevice> &devices, set<string> &deviceKeys) {
    HRESULT initResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool shouldUninitialize = SUCCEEDED(initResult);
    if(FAILED(initResult) && initResult != RPC_E_CHANGED_MODE) return;

    IMMDeviceEnumerator *enumerator = nullptr;
    HRESULT result = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        reinterpret_cast<void **>(&enumerator)
    );

    if(SUCCEEDED(result) && enumerator) {
        IMMDeviceCollection *collection = nullptr;
        result = enumerator->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE, &collection);
        if(SUCCEEDED(result) && collection) {
            UINT count = 0;
            collection->GetCount(&count);

            for(UINT i = 0; i < count; i++) {
                IMMDevice *audioDevice = nullptr;
                if(FAILED(collection->Item(i, &audioDevice)) || !audioDevice) continue;

                LPWSTR id = nullptr;
                string deviceId = "";
                if(SUCCEEDED(audioDevice->GetId(&id)) && id) {
                    deviceId = helpers::wstr2str(id);
                    CoTaskMemFree(id);
                }

                IPropertyStore *properties = nullptr;
                string name = "";
                if(SUCCEEDED(audioDevice->OpenPropertyStore(STGM_READ, &properties)) && properties) {
                    PROPVARIANT friendlyName;
                    PropVariantInit(&friendlyName);
                    if(SUCCEEDED(properties->GetValue(PKEY_Device_FriendlyName, &friendlyName)) &&
                        friendlyName.vt == VT_LPWSTR &&
                        friendlyName.pwszVal) {
                        name = helpers::wstr2str(friendlyName.pwszVal);
                    }
                    PropVariantClear(&friendlyName);
                    properties->Release();
                }

                ConnectedDevice device;
                device.name = name.empty() ? "Audio device" : name;
                device.type = "AUDIO";
                device.vendorId = "";
                device.productId = "";

                string key = "AUDIO|" + (deviceId.empty() ? device.name : deviceId);
                if(deviceKeys.find(key) == deviceKeys.end()) {
                    deviceKeys.insert(key);
                    devices.push_back(device);
                }

                audioDevice->Release();
            }

            collection->Release();
        }

        enumerator->Release();
    }

    if(shouldUninitialize) {
        CoUninitialize();
    }
}

#elif defined(__linux__)

struct __LinuxInputBlock {
    string bus;
    string vendorId;
    string productId;
    string name;
    string phys;
    string sysfs;
    string uniq;
    string handlers;
};

string __extractLinuxInputValue(const string &line) {
    size_t pos = line.find('=');
    if(pos == string::npos) return "";

    string value = __trim(line.substr(pos + 1));
    if(value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        value = value.substr(1, value.size() - 2);
    }
    return value;
}

void __setLinuxInputIds(const string &line, __LinuxInputBlock &block) {
    regex busRegex("Bus=([0-9a-fA-F]+)");
    regex vendorRegex("Vendor=([0-9a-fA-F]+)");
    regex productRegex("Product=([0-9a-fA-F]+)");
    smatch match;

    if(regex_search(line, match, busRegex) && match.size() > 1) {
        block.bus = __formatHexId(match[1].str());
    }
    if(regex_search(line, match, vendorRegex) && match.size() > 1) {
        block.vendorId = __formatHexId(match[1].str());
    }
    if(regex_search(line, match, productRegex) && match.size() > 1) {
        block.productId = __formatHexId(match[1].str());
    }
}

string __getLinuxInputType(const __LinuxInputBlock &block) {
    string loweredName = __toLower(block.name);
    string loweredHandlers = __toLower(block.handlers);

    if(loweredHandlers.find("kbd") != string::npos || loweredName.find("keyboard") != string::npos) {
        return "KEYBOARD";
    }

    if(loweredHandlers.find("mouse") != string::npos ||
        loweredName.find("mouse") != string::npos ||
        loweredName.find("touchpad") != string::npos ||
        loweredName.find("trackpad") != string::npos) {
        return "MOUSE";
    }

    return "HID";
}

bool __isLinuxExternalInputDevice(const __LinuxInputBlock &block) {
    string loweredName = __toLower(block.name);
    string loweredPhys = __toLower(block.phys);
    string loweredSysfs = __toLower(block.sysfs);

    return block.bus == "0003" ||
        block.bus == "0005" ||
        loweredName.find("usb") != string::npos ||
        loweredName.find("bluetooth") != string::npos ||
        loweredPhys.find("usb") != string::npos ||
        loweredSysfs.find("usb") != string::npos ||
        loweredSysfs.find("bluetooth") != string::npos;
}

bool __shouldKeepLinuxInputDevice(const __LinuxInputBlock &block, const string &type) {
    if(type == "MOUSE" || type == "KEYBOARD") return true;
    return __isLinuxExternalInputDevice(block);
}

void __addLinuxInputBlock(vector<ConnectedDevice> &devices, set<string> &deviceKeys, const vector<string> &lines) {
    __LinuxInputBlock block;

    for(const string &line: lines) {
        if(line.rfind("I:", 0) == 0) {
            __setLinuxInputIds(line, block);
        }
        else if(line.rfind("N:", 0) == 0) {
            block.name = __extractLinuxInputValue(line);
        }
        else if(line.rfind("P:", 0) == 0) {
            block.phys = __extractLinuxInputValue(line);
        }
        else if(line.rfind("S:", 0) == 0) {
            block.sysfs = __extractLinuxInputValue(line);
        }
        else if(line.rfind("U:", 0) == 0) {
            block.uniq = __extractLinuxInputValue(line);
        }
        else if(line.rfind("H:", 0) == 0) {
            block.handlers = line;
        }
    }

    string type = __getLinuxInputType(block);
    if(block.handlers.find("event") == string::npos && type == "HID") return;
    if(!__shouldKeepLinuxInputDevice(block, type)) return;

    ConnectedDevice device;
    device.name = block.name;
    device.type = type;
    device.vendorId = block.vendorId;
    device.productId = block.productId;

    __addDevice(devices, deviceKeys, device);
}

void __addLinuxInputDevices(vector<ConnectedDevice> &devices, set<string> &deviceKeys) {
    ifstream inputDevices("/proc/bus/input/devices");
    if(!inputDevices.is_open()) return;

    vector<string> block;
    string line;
    while(getline(inputDevices, line)) {
        if(__trim(line).empty()) {
            if(!block.empty()) {
                __addLinuxInputBlock(devices, deviceKeys, block);
                block.clear();
            }
        }
        else {
            block.push_back(line);
        }
    }

    if(!block.empty()) {
        __addLinuxInputBlock(devices, deviceKeys, block);
    }
}

string __readTextFile(const string &path) {
    ifstream file(path);
    if(!file.is_open()) return "";

    string value;
    getline(file, value);
    return __trim(value);
}

void __setLinuxAudioIds(const string &cardNumber, ConnectedDevice &device) {
    string basePath = "/sys/class/sound/card" + cardNumber + "/device/";
    device.vendorId = __formatHexId(__readTextFile(basePath + "vendor"));
    device.productId = __formatHexId(__readTextFile(basePath + "device"));
}

void __addLinuxAudioDevices(vector<ConnectedDevice> &devices, set<string> &deviceKeys) {
    ifstream cards("/proc/asound/cards");
    if(!cards.is_open()) return;

    regex cardRegex("^\\s*([0-9]+)\\s+\\[([^\\]]+)\\]:\\s*(.+)$");
    string line;
    while(getline(cards, line)) {
        smatch match;
        if(!regex_search(line, match, cardRegex) || match.size() < 4) continue;

        ConnectedDevice device;
        device.type = "AUDIO";
        device.name = __trim(match[3].str());
        if(device.name.empty()) {
            device.name = __trim(match[2].str());
        }
        __setLinuxAudioIds(match[1].str(), device);

        __addDevice(devices, deviceKeys, device);
    }
}

#elif defined(__APPLE__)

string __cfStringToString(CFStringRef value) {
    if(!value) return "";

    CFIndex length = CFStringGetLength(value);
    CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
    string output(maxSize, '\0');

    if(!CFStringGetCString(value, output.data(), maxSize, kCFStringEncodingUTF8)) {
        return "";
    }

    output.resize(strlen(output.c_str()));
    return output;
}

unsigned int __cfNumberToUInt(CFTypeRef value) {
    if(!value || CFGetTypeID(value) != CFNumberGetTypeID()) return 0;

    unsigned int output = 0;
    CFNumberGetValue(static_cast<CFNumberRef>(value), kCFNumberIntType, &output);
    return output;
}

string __getMacHIDType(unsigned int usagePage, unsigned int usage) {
    if(usagePage == kHIDPage_GenericDesktop) {
        if(usage == kHIDUsage_GD_Mouse || usage == kHIDUsage_GD_Pointer) return "MOUSE";
        if(usage == kHIDUsage_GD_Keyboard || usage == kHIDUsage_GD_Keypad) return "KEYBOARD";
    }
    return "HID";
}

bool __isMacExternalHIDTransport(const string &transport) {
    string loweredTransport = __toLower(transport);
    return loweredTransport.find("usb") != string::npos ||
        loweredTransport.find("bluetooth") != string::npos;
}

bool __shouldKeepMacHIDDevice(const string &type, const string &transport) {
    if(type == "MOUSE" || type == "KEYBOARD") return true;
    return __isMacExternalHIDTransport(transport);
}

string __getMacHIDGroupKey(IOHIDDeviceRef hidDevice, const ConnectedDevice &device, const string &transport) {
    unsigned int locationId = __cfNumberToUInt(
        IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDLocationIDKey))
    );
    if(locationId != 0) {
        return __formatHexId(locationId) + "|" + device.type;
    }
    return transport + "|" + device.vendorId + "|" + device.productId + "|" + device.name + "|" + device.type;
}

void __addMacHIDDevices(vector<ConnectedDevice> &devices, set<string> &deviceKeys) {
    IOHIDManagerRef manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if(!manager) return;

    IOHIDManagerSetDeviceMatching(manager, nullptr);
    CFSetRef deviceSet = IOHIDManagerCopyDevices(manager);
    if(deviceSet) {
        map<string, ConnectedDevice> selectedDevices;
        CFIndex deviceCount = CFSetGetCount(deviceSet);
        vector<const void*> hidDeviceValues(deviceCount);
        CFSetGetValues(deviceSet, hidDeviceValues.data());

        for(const void *hidDeviceValue: hidDeviceValues) {
            IOHIDDeviceRef hidDevice = static_cast<IOHIDDeviceRef>(const_cast<void*>(hidDeviceValue));
            ConnectedDevice device;
            device.name = __cfStringToString(static_cast<CFStringRef>(
                IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDProductKey))
            ));
            device.vendorId = __formatHexId(__cfNumberToUInt(
                IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDVendorIDKey))
            ));
            device.productId = __formatHexId(__cfNumberToUInt(
                IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDProductIDKey))
            ));

            unsigned int usagePage = __cfNumberToUInt(
                IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDPrimaryUsagePageKey))
            );
            unsigned int usage = __cfNumberToUInt(
                IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDPrimaryUsageKey))
            );
            device.type = __getMacHIDType(usagePage, usage);

            string transport = __cfStringToString(static_cast<CFStringRef>(
                IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDTransportKey))
            ));
            if(!__shouldKeepMacHIDDevice(device.type, transport)) continue;

            selectedDevices[__getMacHIDGroupKey(hidDevice, device, transport)] = device;
        }

        for(const auto &[groupKey, device]: selectedDevices) {
            __addDevice(devices, deviceKeys, device);
        }

        CFRelease(deviceSet);
    }

    CFRelease(manager);
}

string __getMacAudioStringProperty(AudioObjectID objectId, AudioObjectPropertySelector selector) {
    AudioObjectPropertyAddress address {
        selector,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };

    CFStringRef value = nullptr;
    UInt32 size = sizeof(value);
    if(AudioObjectGetPropertyData(objectId, &address, 0, nullptr, &size, &value) != noErr || !value) {
        return "";
    }

    string output = __cfStringToString(value);
    CFRelease(value);
    return output;
}

bool __macAudioDeviceHasStreams(AudioObjectID deviceId, AudioObjectPropertyScope scope) {
    AudioObjectPropertyAddress address {
        kAudioDevicePropertyStreams,
        scope,
        kAudioObjectPropertyElementMaster
    };

    UInt32 size = 0;
    return AudioObjectGetPropertyDataSize(deviceId, &address, 0, nullptr, &size) == noErr && size > 0;
}

void __addMacAudioDevices(vector<ConnectedDevice> &devices, set<string> &deviceKeys) {
    AudioObjectPropertyAddress address {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };

    UInt32 size = 0;
    if(AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &address, 0, nullptr, &size) != noErr || size == 0) {
        return;
    }

    vector<AudioObjectID> audioDevices(size / sizeof(AudioObjectID));
    if(AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, nullptr, &size, audioDevices.data()) != noErr) {
        return;
    }

    for(AudioObjectID audioDevice: audioDevices) {
        if(!__macAudioDeviceHasStreams(audioDevice, kAudioDevicePropertyScopeInput) &&
            !__macAudioDeviceHasStreams(audioDevice, kAudioDevicePropertyScopeOutput)) {
            continue;
        }

        ConnectedDevice device;
        device.type = "AUDIO";
        device.name = __getMacAudioStringProperty(audioDevice, kAudioObjectPropertyName);
        device.vendorId = "";
        device.productId = "";

        string uid = __getMacAudioStringProperty(audioDevice, kAudioDevicePropertyDeviceUID);
        string key = "AUDIO|" + (uid.empty() ? device.name : uid);
        if(deviceKeys.find(key) == deviceKeys.end()) {
            deviceKeys.insert(key);
            devices.push_back(device);
        }
    }
}

#endif

} // namespace

vector<ConnectedDevice> getConnectedDevices() {
    vector<ConnectedDevice> devices;
    set<string> deviceKeys;

    #if defined(_WIN32)
    __addWindowsInputDevices(devices, deviceKeys);
    __addWindowsAudioDevices(devices, deviceKeys);
    #elif defined(__linux__)
    __addLinuxInputDevices(devices, deviceKeys);
    __addLinuxAudioDevices(devices, deviceKeys);
    #elif defined(__APPLE__)
    __addMacHIDDevices(devices, deviceKeys);
    __addMacAudioDevices(devices, deviceKeys);
    #endif

    return devices;
}

} // namespace computer
