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
#include <cstdlib>
#include <cstring>
#include <sys/utsname.h>
#include <fstream>

#ifdef __APPLE__
	#include <CoreFoundation/CoreFoundation.h>
	#include <IOKit/IOKitLib.h>
#endif

iware::system::kernel_info_t iware::system::kernel_info() {
	utsname uts;
	uname(&uts);

	char* marker                     = uts.release;
	const std::uint32_t major        = std::strtoul(marker, &marker, 10);
	const std::uint32_t minor        = std::strtoul(marker + 1, &marker, 10);
	const std::uint32_t patch        = std::strtoul(marker + 1, &marker, 10);
	const std::uint32_t build_number = std::strtoul(marker + 1, nullptr, 10);

	auto kernel = iware::system::kernel_t::unknown;
	if(!std::strcmp(uts.sysname, "Linux"))
		kernel = iware::system::kernel_t::linux;
	else if(!std::strcmp(uts.sysname, "Darwin"))
		kernel = iware::system::kernel_t::darwin;

	return {kernel, major, minor, patch, build_number};
}

std::string iware::system::machine_uuid() {
#if defined(__linux__)

	std::ifstream file("/etc/machine-id");

	if(!file.is_open())
		return "";

	std::string machine_id;
	std::getline(file, machine_id);

	return machine_id;

#elif defined(__APPLE__)

	io_service_t service = IOServiceGetMatchingService(
	    kIOMainPortDefault,
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

	char buffer[256];

	const auto success = CFStringGetCString(
	    (CFStringRef)uuid_cf,
	    buffer,
	    sizeof(buffer),
	    kCFStringEncodingUTF8);

	CFRelease(uuid_cf);

	if(!success)
		return "";

	return buffer;

#else

	return "";

#endif
}

#endif
