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


#ifndef _WIN32


#include "infoware/system.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <sys/utsname.h>

#ifdef __APPLE__
	#include <CoreFoundation/CoreFoundation.h>
	#include <IOKit/IOKitLib.h>
#endif

using namespace std;

namespace {
	string trim(string value) {
		value.erase(value.begin(), find_if(value.begin(), value.end(), [](unsigned char ch) {
			return !isspace(ch);
		}));
		value.erase(find_if(value.rbegin(), value.rend(), [](unsigned char ch) {
			return !isspace(ch);
		}).base(), value.end());
		return value;
	}

	bool is_hex_string(const string& value) {
		return all_of(value.begin(), value.end(), [](unsigned char ch) {
			return isxdigit(ch);
		});
	}

	bool is_zero_string(const string& value) {
		return all_of(value.begin(), value.end(), [](char ch) {
			return ch == '0';
		});
	}

	bool is_linux_machine_id(const string& value) {
		return value.size() == 32 && is_hex_string(value) && !is_zero_string(value);
	}

	bool is_macos_uuid(const string& value) {
		constexpr size_t hyphen_positions[] = {8, 13, 18, 23};

		if(value.size() != 36)
			return false;

		for(size_t i = 0; i < value.size(); ++i) {
			const auto is_hyphen_position =
			    find(begin(hyphen_positions), end(hyphen_positions), i) != end(hyphen_positions);
			if(is_hyphen_position) {
				if(value[i] != '-')
					return false;
			}
			else if(!isxdigit(static_cast<unsigned char>(value[i]))) {
				return false;
			}
		}
		return true;
	}

	string read_linux_machine_id(const char* path) {
		ifstream file(path);
		if(!file.is_open())
			return "";

		string machine_id;
		getline(file, machine_id);
		machine_id = trim(machine_id);

		if(!is_linux_machine_id(machine_id))
			return "";

		transform(machine_id.begin(), machine_id.end(), machine_id.begin(), [](unsigned char ch) {
			return static_cast<char>(tolower(ch));
		});
		return machine_id;
	}
}

iware::system::kernel_info_t iware::system::kernel_info() {
	utsname uts;
	uname(&uts);

	char* marker                     = uts.release;
	const uint32_t major        = strtoul(marker, &marker, 10);
	const uint32_t minor        = strtoul(marker + 1, &marker, 10);
	const uint32_t patch        = strtoul(marker + 1, &marker, 10);
	const uint32_t build_number = strtoul(marker + 1, nullptr, 10);

	auto kernel = iware::system::kernel_t::unknown;
	if(!strcmp(uts.sysname, "Linux"))
		kernel = iware::system::kernel_t::linux;
	else if(!strcmp(uts.sysname, "Darwin"))
		kernel = iware::system::kernel_t::darwin;

	return {kernel, major, minor, patch, build_number};
}

string iware::system::machine_uuid() {
#if defined(__linux__)

	const auto etc_machine_id = read_linux_machine_id("/etc/machine-id");
	if(!etc_machine_id.empty())
		return etc_machine_id;

	return read_linux_machine_id("/var/lib/dbus/machine-id");

#elif defined(__APPLE__)

	io_service_t service = IOServiceGetMatchingService(
#if defined(kIOMainPortDefault)
	    kIOMainPortDefault,
#else
	    kIOMasterPortDefault,
#endif
	    IOServiceMatching("IOPlatformExpertDevice"));

	if(!service)
		return "";

	CFTypeRef uuid_cf = IORegistryEntryCreateCFProperty(
	    service,
	    CFSTR("IOPlatformUUID"),
	    kCFAllocatorDefault,
	    0);

	IOObjectRelease(service);

	if(!uuid_cf)
		return "";

	if(CFGetTypeID(uuid_cf) != CFStringGetTypeID()) {
		CFRelease(uuid_cf);
		return "";
	}

	char buffer[64] = {0};

	const auto success = CFStringGetCString(
	    (CFStringRef)uuid_cf,
	    buffer,
	    sizeof(buffer),
	    kCFStringEncodingUTF8);

	CFRelease(uuid_cf);

	if(!success)
		return "";

	const auto uuid = trim(buffer);
	if(!is_macos_uuid(uuid))
		return "";

	return uuid;

#else

	return "";

#endif
}

#endif
