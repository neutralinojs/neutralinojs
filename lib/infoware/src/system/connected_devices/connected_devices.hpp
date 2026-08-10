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


#pragma once


#include "infoware/system.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <set>
#include <sstream>


namespace iware {
	namespace system {
		namespace detail {
			inline std::string to_lower(std::string value) {
				std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
					return static_cast<char>(std::tolower(ch));
				});
				return value;
			}

			inline std::string trim(std::string value) {
				const auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
					return std::isspace(ch);
				});
				const auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
					return std::isspace(ch);
				}).base();

				if(begin >= end)
					return "";
				return std::string(begin, end);
			}

			inline std::string format_hex_id(std::uint32_t value) {
				if(value == 0)
					return "";

				std::ostringstream stream;
				stream << std::hex << std::nouppercase << std::setw(4) << std::setfill('0') << value;
				return stream.str();
			}

			inline std::string normalize_hex_id(std::string value) {
				value = trim(value);
				if(value.size() >= 2 && value[0] == '0' && (value[1] == 'x' || value[1] == 'X'))
					value = value.substr(2);

				if(value.empty())
					return "";

				for(const char ch : value) {
					if(!std::isxdigit(static_cast<unsigned char>(ch)))
						return "";
				}

				value = to_lower(value);
				while(value.size() > 4 && value.front() == '0')
					value.erase(value.begin());

				const bool has_non_zero_digit = std::any_of(value.begin(), value.end(), [](char ch) {
					return ch != '0';
				});
				if(!has_non_zero_digit)
					return "";

				if(value.size() < 4)
					value.insert(value.begin(), 4 - value.size(), '0');
				return value;
			}

			inline connected_device_type_t hid_usage_type(std::uint32_t usage_page, std::uint32_t usage) {
				if(usage_page == 0x01) {
					if(usage == 0x02)
						return connected_device_type_t::mouse;
					if(usage == 0x04)
						return connected_device_type_t::joystick;
					if(usage == 0x05)
						return connected_device_type_t::gamepad;
					if(usage == 0x06 || usage == 0x07)
						return connected_device_type_t::keyboard;
				}
				if(usage_page == 0x0d) {
					if(usage == 0x02)
						return connected_device_type_t::pen;
					if(usage == 0x04)
						return connected_device_type_t::touchscreen;
					if(usage == 0x05)
						return connected_device_type_t::touchpad;
				}
				return connected_device_type_t::hid;
			}

			inline std::string fallback_device_name(connected_device_type_t type) {
				switch(type) {
					case connected_device_type_t::mouse:
						return "Mouse";
					case connected_device_type_t::keyboard:
						return "Keyboard";
					case connected_device_type_t::touchpad:
						return "Touchpad";
					case connected_device_type_t::touchscreen:
						return "Touchscreen";
					case connected_device_type_t::gamepad:
						return "Gamepad";
					case connected_device_type_t::joystick:
						return "Joystick";
					case connected_device_type_t::pen:
						return "Pen";
					case connected_device_type_t::hid:
						return "HID Device";
					case connected_device_type_t::unknown:
						return "";
				}
				return "";
			}

			inline void add_connected_device(std::vector<connected_device_t>& devices, std::set<std::string>& seen_devices,
			                                std::string name, connected_device_type_t type, std::string vendor_id,
			                                std::string product_id) {
				if(type == connected_device_type_t::unknown)
					return;

				name       = trim(name);
				vendor_id  = normalize_hex_id(vendor_id);
				product_id = normalize_hex_id(product_id);
				if(name.empty())
					name = fallback_device_name(type);
				if(name.empty())
					return;

				const std::string key =
				    to_lower(name) + "|" + std::to_string(static_cast<int>(type)) + "|" + vendor_id + "|" + product_id;
				if(!seen_devices.insert(key).second)
					return;

				devices.push_back({name, type, vendor_id, product_id});
			}
		}  // namespace detail
	}  // namespace system
}  // namespace iware
