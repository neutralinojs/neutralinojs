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


#ifdef _WIN32


#include "connected_devices.hpp"

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <hidsdi.h>

#include <cwctype>
#include <set>
#include <vector>


namespace {
	std::string wide_to_utf8(const std::wstring& value) {
		if(value.empty())
			return "";

		const int input_size = static_cast<int>(value.size());
		const int size       = WideCharToMultiByte(CP_UTF8, 0, value.data(), input_size, nullptr, 0, nullptr, nullptr);
		if(size <= 0)
			return "";

		std::string result(static_cast<std::size_t>(size), '\0');
		WideCharToMultiByte(CP_UTF8, 0, value.data(), input_size, &result[0], size, nullptr, nullptr);
		return result;
	}

	std::string extract_device_id(std::wstring device_path, const std::wstring& key) {
		std::transform(device_path.begin(), device_path.end(), device_path.begin(), [](wchar_t ch) {
			return static_cast<wchar_t>(std::towlower(ch));
		});

		std::size_t offset = device_path.find(key);
		if(offset == std::wstring::npos)
			return "";

		offset += key.size();
		std::wstring value;
		while(offset < device_path.size() && std::iswxdigit(device_path[offset])) {
			value.push_back(device_path[offset]);
			offset++;
		}

		return iware::system::detail::normalize_hex_id(wide_to_utf8(value));
	}

	std::wstring raw_input_device_path(HANDLE handle) {
		UINT size = 0;
		if(GetRawInputDeviceInfoW(handle, RIDI_DEVICENAME, nullptr, &size) == static_cast<UINT>(-1))
			return L"";
		if(size == 0)
			return L"";

		std::wstring device_path(size, L'\0');
		if(GetRawInputDeviceInfoW(handle, RIDI_DEVICENAME, &device_path[0], &size) == static_cast<UINT>(-1))
			return L"";

		while(!device_path.empty() && device_path.back() == L'\0')
			device_path.pop_back();

		return device_path;
	}

	std::string hid_product_name(const std::wstring& device_path) {
		if(device_path.empty())
			return "";

		HANDLE file = CreateFileW(device_path.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0,
		                          nullptr);
		if(file == INVALID_HANDLE_VALUE)
			return "";

		wchar_t product_name[256] = {0};
		const bool success        = HidD_GetProductString(file, product_name, sizeof(product_name));
		CloseHandle(file);

		if(!success)
			return "";
		return wide_to_utf8(product_name);
	}
}  // namespace


std::vector<iware::system::connected_device_t> iware::system::connected_devices() {
	std::vector<iware::system::connected_device_t> devices;
	std::set<std::string> seen_devices;

	UINT device_count = 0;
	if(GetRawInputDeviceList(nullptr, &device_count, sizeof(RAWINPUTDEVICELIST)) == static_cast<UINT>(-1))
		return devices;
	if(device_count == 0)
		return devices;

	std::vector<RAWINPUTDEVICELIST> raw_devices(device_count);
	const UINT listed_devices = GetRawInputDeviceList(raw_devices.data(), &device_count, sizeof(RAWINPUTDEVICELIST));
	if(listed_devices == static_cast<UINT>(-1))
		return devices;

	for(UINT i = 0; i < listed_devices; i++) {
		const RAWINPUTDEVICELIST& raw_device = raw_devices[i];
		RID_DEVICE_INFO device_info{};
		device_info.cbSize      = sizeof(RID_DEVICE_INFO);
		UINT device_info_size   = sizeof(RID_DEVICE_INFO);
		const auto device_state = GetRawInputDeviceInfoW(raw_device.hDevice, RIDI_DEVICEINFO, &device_info, &device_info_size);
		if(device_state == static_cast<UINT>(-1))
			continue;

		connected_device_type_t type = connected_device_type_t::unknown;
		std::string vendor_id;
		std::string product_id;

		if(raw_device.dwType == RIM_TYPEMOUSE) {
			type = connected_device_type_t::mouse;
		} else if(raw_device.dwType == RIM_TYPEKEYBOARD) {
			type = connected_device_type_t::keyboard;
		} else if(raw_device.dwType == RIM_TYPEHID) {
			type       = detail::hid_usage_type(device_info.hid.usUsagePage, device_info.hid.usUsage);
			vendor_id  = detail::format_hex_id(device_info.hid.dwVendorId);
			product_id = detail::format_hex_id(device_info.hid.dwProductId);
		}

		const std::wstring device_path = raw_input_device_path(raw_device.hDevice);
		if(vendor_id.empty())
			vendor_id = extract_device_id(device_path, L"vid_");
		if(product_id.empty())
			product_id = extract_device_id(device_path, L"pid_");

		detail::add_connected_device(devices, seen_devices, hid_product_name(device_path), type, vendor_id, product_id);
	}

	return devices;
}


#endif
