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
#include <string>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#endif

using namespace std;


#ifdef __linux__
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
static bool cfDataToUint32(CFTypeRef value, uint32_t &output) {
	if(!value || CFGetTypeID(value) != CFDataGetTypeID())
		return false;

	const auto data = static_cast<CFDataRef>(value);
	if(CFDataGetLength(data) < static_cast<CFIndex>(sizeof(output)))
		return false;

	memcpy(&output, CFDataGetBytePtr(data), sizeof(output));
	return true;
}


static string cfDataToString(CFTypeRef value) {
	if(!value || CFGetTypeID(value) != CFDataGetTypeID())
		return "";

	const auto data = static_cast<CFDataRef>(value);
	string output(reinterpret_cast<const char*>(CFDataGetBytePtr(data)), CFDataGetLength(data));
	output.erase(find(output.begin(), output.end(), '\0'), output.end());
	return output;
}


static size_t cfNumberToSize(CFTypeRef value) {
	if(!value || CFGetTypeID(value) != CFNumberGetTypeID())
		return 0;

	uint64_t output = 0;
	CFNumberGetValue(static_cast<CFNumberRef>(value), kCFNumberSInt64Type, &output);
	return static_cast<size_t>(output);
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


static bool isDisplayController(io_service_t service) {
	CFTypeRef classCodeRef = IORegistryEntryCreateCFProperty(service, CFSTR("class-code"), kCFAllocatorDefault, 0);
	uint32_t classCode = 0;
	const bool hasClassCode = cfDataToUint32(classCodeRef, classCode);
	if(classCodeRef)
		CFRelease(classCodeRef);

	return hasClassCode && ((classCode >> 16) & 0xFF) == 0x03;
}
#endif


vector<iware::gpu::device_properties_t> iware::gpu::device_properties() {
#ifdef __linux__
	vector<iware::gpu::device_properties_t> devices{};
	for(auto cardId = 0u; cardId < 16; ++cardId) {
		const auto devicePath = "/sys/class/drm/card" + to_string(cardId) + "/device/";

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
	if(IOServiceGetMatchingServices(kIOMainPortDefault, matchDict, &iterator) != KERN_SUCCESS)
		return {};

	vector<iware::gpu::device_properties_t> devices{};
	io_service_t service;
	while((service = IOIteratorNext(iterator))) {
		if(!isDisplayController(service)) {
			IOObjectRelease(service);
			continue;
		}

		CFTypeRef vendorIdRef = IORegistryEntryCreateCFProperty(service, CFSTR("vendor-id"), kCFAllocatorDefault, 0);
		CFTypeRef modelRef = IORegistryEntryCreateCFProperty(service, CFSTR("model"), kCFAllocatorDefault, 0);
		CFTypeRef memoryRef = IORegistryEntryCreateCFProperty(service, CFSTR("VRAM,totalMB"), kCFAllocatorDefault, 0);

		uint32_t vendorId = 0;
		cfDataToUint32(vendorIdRef, vendorId);
		const auto memorySize = cfNumberToSize(memoryRef) * 1024 * 1024;

		devices.push_back({vendorFromId(vendorId), cfDataToString(modelRef), memorySize, 0, 0});

		if(vendorIdRef)
			CFRelease(vendorIdRef);
		if(modelRef)
			CFRelease(modelRef);
		if(memoryRef)
			CFRelease(memoryRef);
		IOObjectRelease(service);
	}

	IOObjectRelease(iterator);
	return devices;
#else
	return {};
#endif
}


#endif
#endif
#endif
