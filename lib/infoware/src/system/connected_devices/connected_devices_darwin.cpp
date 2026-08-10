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


#ifdef __APPLE__


#include "connected_devices.hpp"

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDManager.h>

#include <set>
#include <vector>


namespace {
	std::string cf_string_to_string(CFTypeRef value) {
		if(!value || CFGetTypeID(value) != CFStringGetTypeID())
			return "";

		CFStringRef string_ref = static_cast<CFStringRef>(value);
		const CFIndex size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(string_ref), kCFStringEncodingUTF8) + 1;
		if(size <= 1)
			return "";

		std::vector<char> buffer(static_cast<std::size_t>(size), '\0');
		if(!CFStringGetCString(string_ref, buffer.data(), size, kCFStringEncodingUTF8))
			return "";

		return std::string(buffer.data());
	}

	std::uint32_t cf_number_to_uint(CFTypeRef value) {
		if(!value || CFGetTypeID(value) != CFNumberGetTypeID())
			return 0;

		long long number = 0;
		if(!CFNumberGetValue(static_cast<CFNumberRef>(value), kCFNumberLongLongType, &number))
			return 0;
		if(number < 0)
			return 0;
		return static_cast<std::uint32_t>(number);
	}
}  // namespace


std::vector<iware::system::connected_device_t> iware::system::connected_devices() {
	std::vector<iware::system::connected_device_t> devices;
	std::set<std::string> seen_devices;

	IOHIDManagerRef manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	if(!manager)
		return devices;

	IOHIDManagerSetDeviceMatching(manager, nullptr);
	if(IOHIDManagerOpen(manager, kIOHIDOptionsTypeNone) != kIOReturnSuccess) {
		CFRelease(manager);
		return devices;
	}

	CFSetRef device_set = IOHIDManagerCopyDevices(manager);
	if(device_set) {
		const CFIndex device_count = CFSetGetCount(device_set);
		if(device_count > 0) {
			std::vector<const void*> device_refs(static_cast<std::size_t>(device_count));
			CFSetGetValues(device_set, device_refs.data());

			for(const void* device_ref : device_refs) {
				IOHIDDeviceRef device = reinterpret_cast<IOHIDDeviceRef>(const_cast<void*>(device_ref));
				std::uint32_t usage_page =
				    cf_number_to_uint(IOHIDDeviceGetProperty(device, CFSTR(kIOHIDPrimaryUsagePageKey)));
				std::uint32_t usage = cf_number_to_uint(IOHIDDeviceGetProperty(device, CFSTR(kIOHIDPrimaryUsageKey)));

				CFTypeRef usage_pairs = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDDeviceUsagePairsKey));
				if((usage_page == 0 || usage == 0) && usage_pairs && CFGetTypeID(usage_pairs) == CFArrayGetTypeID() &&
				   CFArrayGetCount(static_cast<CFArrayRef>(usage_pairs)) > 0) {
					CFTypeRef usage_pair = CFArrayGetValueAtIndex(static_cast<CFArrayRef>(usage_pairs), 0);
					if(usage_pair && CFGetTypeID(usage_pair) == CFDictionaryGetTypeID()) {
						CFDictionaryRef usage_pair_dict = static_cast<CFDictionaryRef>(usage_pair);
						usage_page =
						    cf_number_to_uint(CFDictionaryGetValue(usage_pair_dict, CFSTR(kIOHIDDeviceUsagePageKey)));
						usage = cf_number_to_uint(CFDictionaryGetValue(usage_pair_dict, CFSTR(kIOHIDDeviceUsageKey)));
					}
				}

				detail::add_connected_device(
				    devices, seen_devices, cf_string_to_string(IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey))),
				    detail::hid_usage_type(usage_page, usage),
				    detail::format_hex_id(cf_number_to_uint(IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey)))),
				    detail::format_hex_id(cf_number_to_uint(IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey)))));
			}
		}

		CFRelease(device_set);
	}

	IOHIDManagerClose(manager, kIOHIDOptionsTypeNone);
	CFRelease(manager);
	return devices;
}


#endif
