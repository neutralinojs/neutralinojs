// infoware - C++ System information Library
//
// Written in 2016-2020 by nabijaczleweli <nabijaczleweli@gmail.com> and ThePhD <phdofthehouse@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright and related
// and neighboring rights to this software to the public domain worldwide. This software is
// distributed without any warranty.
//
// You should have received a copy of the CC0 Public Domain Dedication along with this software.
// If not, see <http://creativecommons.org/publicdomain/zero/1.0/>


#ifndef INFOWARE_USE_OPENGL
#ifndef INFOWARE_USE_OPENCL
#ifndef INFOWARE_USE_D3D


#include "infoware/gpu.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <set>
#include <string>

#ifdef __linux__
#include <dirent.h>
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#endif

using namespace std;


#ifdef __linux__
static vector<string> drmCardDevicePaths() {
	vector<string> devicePaths{};
	DIR* drmDir = opendir("/sys/class/drm");
	if(!drmDir)
		return devicePaths;

	dirent* entry;
	while((entry = readdir(drmDir))) {
		const string name = entry->d_name;
		if(name.find("card") != 0 || name.find("-") != string::npos)
			continue;

		const auto id = name.substr(4);
		if(id.empty() || !all_of(id.begin(), id.end(), [](const char ch) { return ch >= '0' && ch <= '9'; }))
			continue;

		devicePaths.push_back("/sys/class/drm/" + name + "/device/");
	}
	closedir(drmDir);

	sort(devicePaths.begin(), devicePaths.end());
	return devicePaths;
}


static bool readLine(const string &path, string &value) {
	ifstream file(path);
	if(!file.is_open())
		return false;

	getline(file, value);
	return true;
}


static bool readUint64(const string &path, uint64_t &value) {
	ifstream file(path);
	if(!file.is_open())
		return false;

	file >> value;
	return true;
}


static uint64_t parseHexId(const string &value) {
	try {
		return stoull(value, nullptr, 16);
	}
	catch(...) {
		return 0;
	}
}


static iware::gpu::vendor_t vendorFromId(const uint64_t id) {
	switch(id) {
		case 0x8086:
			return iware::gpu::vendor_t::intel;
		case 0x1002:
		case 0x1022:
			return iware::gpu::vendor_t::amd;
		case 0x10DE:
			return iware::gpu::vendor_t::nvidia;
		case 0x1414:
			return iware::gpu::vendor_t::microsoft;
		case 0x5143:
			return iware::gpu::vendor_t::qualcomm;
		default:
			return iware::gpu::vendor_t::unknown;
	}
}


static string readDeviceName(const string &devicePath) {
	string name;
	if(readLine(devicePath + "product_name", name))
		return name;

	ifstream file(devicePath + "uevent");
	if(!file.is_open())
		return "";

	string line;
	while(getline(file, line)) {
		const string driverPrefix = "DRIVER=";
		if(line.find(driverPrefix) == 0)
			return line.substr(driverPrefix.size());
	}
	return "";
}
#endif


#ifdef __APPLE__
static string cfStringToString(CFStringRef value) {
	if(!value)
		return "";

	const char* directString = CFStringGetCStringPtr(value, kCFStringEncodingUTF8);
	if(directString)
		return directString;

	const CFIndex length = CFStringGetLength(value);
	const CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
	string output(static_cast<size_t>(maxSize), '\0');
	if(!CFStringGetCString(value, &output[0], maxSize, kCFStringEncodingUTF8))
		return "";

	output.erase(find(output.begin(), output.end(), '\0'), output.end());
	return output;
}


static bool cfDataToUint32(CFTypeRef value, uint32_t &output) {
	if(!value || CFGetTypeID(value) != CFDataGetTypeID())
		return false;

	const auto data = static_cast<CFDataRef>(value);
	if(CFDataGetLength(data) < static_cast<CFIndex>(sizeof(output)))
		return false;

	memcpy(&output, CFDataGetBytePtr(data), sizeof(output));
	return true;
}


static bool cfDataToUint64(CFTypeRef value, uint64_t &output) {
	if(!value || CFGetTypeID(value) != CFDataGetTypeID())
		return false;

	const auto data = static_cast<CFDataRef>(value);
	const auto length = min<CFIndex>(CFDataGetLength(data), static_cast<CFIndex>(sizeof(output)));
	if(length <= 0)
		return false;

	output = 0;
	memcpy(&output, CFDataGetBytePtr(data), length);
	return true;
}


static string cfTypeToString(CFTypeRef value) {
	if(!value)
		return "";

	if(CFGetTypeID(value) == CFStringGetTypeID())
		return cfStringToString(static_cast<CFStringRef>(value));

	if(CFGetTypeID(value) == CFDataGetTypeID()) {
		const auto data = static_cast<CFDataRef>(value);
		string output(reinterpret_cast<const char*>(CFDataGetBytePtr(data)), CFDataGetLength(data));
		output.erase(find(output.begin(), output.end(), '\0'), output.end());
		return output;
	}

	return "";
}


static bool cfNumberToUint64(CFTypeRef value, uint64_t &output) {
	if(!value || CFGetTypeID(value) != CFNumberGetTypeID())
		return false;

	return CFNumberGetValue(static_cast<CFNumberRef>(value), kCFNumberSInt64Type, &output);
}


static bool cfTypeToUint64(CFTypeRef value, uint64_t &output) {
	return cfNumberToUint64(value, output) || cfDataToUint64(value, output);
}


static bool readRegistryUint64(io_service_t service, CFStringRef key, uint64_t &output) {
	CFTypeRef value = IORegistryEntryCreateCFProperty(service, key, kCFAllocatorDefault, 0);
	const bool hasValue = cfTypeToUint64(value, output);
	if(value)
		CFRelease(value);
	return hasValue;
}


static string readRegistryString(io_service_t service, CFStringRef key) {
	CFTypeRef value = IORegistryEntryCreateCFProperty(service, key, kCFAllocatorDefault, 0);
	const auto output = cfTypeToString(value);
	if(value)
		CFRelease(value);
	return output;
}


static size_t readMemorySize(io_service_t service) {
	uint64_t output = 0;
	if(readRegistryUint64(service, CFSTR("VRAM,totalsize"), output))
		return static_cast<size_t>(output);

	if(readRegistryUint64(service, CFSTR("VRAM,totalMB"), output))
		return static_cast<size_t>(output * 1024 * 1024);

	return 0;
}


static string readDeviceName(io_service_t service) {
	string name = readRegistryString(service, CFSTR("model"));
	if(!name.empty())
		return name;

	name = readRegistryString(service, CFSTR("IOName"));
	if(!name.empty())
		return name;

	char entryName[128] = {0};
	if(IORegistryEntryGetName(service, entryName) == KERN_SUCCESS)
		return entryName;

	return "";
}


static iware::gpu::vendor_t vendorFromId(const uint32_t id) {
	switch(id) {
		case 0x8086:
			return iware::gpu::vendor_t::intel;
		case 0x1002:
		case 0x1022:
			return iware::gpu::vendor_t::amd;
		case 0x10DE:
			return iware::gpu::vendor_t::nvidia;
		case 0x106B:
			return iware::gpu::vendor_t::apple;
		case 0x5143:
			return iware::gpu::vendor_t::qualcomm;
		default:
			return iware::gpu::vendor_t::unknown;
	}
}


static void appendDevice(vector<iware::gpu::device_properties_t> &devices, set<string> &deviceNames, const iware::gpu::vendor_t vendor,
                         const string &name, const size_t memorySize) {
	if(name.empty() || deviceNames.find(name) != deviceNames.end())
		return;

	deviceNames.insert(name);
	devices.push_back({vendor, name, memorySize, 0, 0});
}


static bool isDisplayController(io_service_t service) {
	CFTypeRef classCodeRef = IORegistryEntryCreateCFProperty(service, CFSTR("class-code"), kCFAllocatorDefault, 0);
	uint32_t classCode = 0;
	const bool hasClassCode = cfDataToUint32(classCodeRef, classCode);
	if(classCodeRef)
		CFRelease(classCodeRef);

	return hasClassCode && ((classCode >> 16) & 0xFF) == 0x03;
}


static void appendIOAcceleratorDevices(vector<iware::gpu::device_properties_t> &devices, set<string> &deviceNames) {
	CFMutableDictionaryRef matchDict = IOServiceMatching("IOAccelerator");
	if(!matchDict)
		return;

	io_iterator_t iterator;
	if(IOServiceGetMatchingServices(kIOMasterPortDefault, matchDict, &iterator) != KERN_SUCCESS)
		return;

	io_service_t service;
	while((service = IOIteratorNext(iterator))) {
		appendDevice(devices, deviceNames, iware::gpu::vendor_t::apple, readDeviceName(service), readMemorySize(service));
		IOObjectRelease(service);
	}

	IOObjectRelease(iterator);
}


#endif


vector<iware::gpu::device_properties_t> iware::gpu::device_properties() {
#ifdef __linux__
	vector<iware::gpu::device_properties_t> devices{};
	for(const auto &devicePath: drmCardDevicePaths()) {
		string vendorIdValue;
		if(!readLine(devicePath + "vendor", vendorIdValue))
			continue;

		uint64_t memorySize = 0;
		readUint64(devicePath + "mem_info_vram_total", memorySize);

		devices.push_back({vendorFromId(parseHexId(vendorIdValue)), readDeviceName(devicePath), static_cast<size_t>(memorySize), 0, 0});
	}
	return devices;
#elif defined(__APPLE__)
	CFMutableDictionaryRef matchDict = IOServiceMatching("IOPCIDevice");
	if(!matchDict)
		return {};

	io_iterator_t iterator;
	if(IOServiceGetMatchingServices(kIOMasterPortDefault, matchDict, &iterator) != KERN_SUCCESS)
		return {};

	vector<iware::gpu::device_properties_t> devices{};
	set<string> deviceNames{};
	io_service_t service;
	while((service = IOIteratorNext(iterator))) {
		if(!isDisplayController(service)) {
			IOObjectRelease(service);
			continue;
		}

		CFTypeRef vendorIdRef = IORegistryEntryCreateCFProperty(service, CFSTR("vendor-id"), kCFAllocatorDefault, 0);

		uint32_t vendorId = 0;
		const bool hasVendorId = cfDataToUint32(vendorIdRef, vendorId);
		if(hasVendorId)
			appendDevice(devices, deviceNames, vendorFromId(vendorId), readDeviceName(service), readMemorySize(service));

		if(vendorIdRef)
			CFRelease(vendorIdRef);
		IOObjectRelease(service);
	}

	IOObjectRelease(iterator);
	appendIOAcceleratorDevices(devices, deviceNames);
	return devices;
#else
	return {};
#endif
}


#endif
#endif
#endif
