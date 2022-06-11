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


#include <infoware/linkage.hpp>

#include <cstdint>
#include <string>


namespace iware {
	namespace pci {
		struct device {
			const char* vendor_name;
			const char* device_name;
		};


		/// Get the names for the device with the specified PCI ID from the vendor with the specified PCI ID.
		///
		/// If the vendor was not found, both names are nullptr.
		/// If the vendor was found, but the device wasn't, the device name is nullptr;
		INFOWARE_API_LINKAGE device identify_device(std::uint64_t vendor_id, std::uint64_t device_id) noexcept;

		/// Get the name the vendor with the specified PCI ID, or nullptr if not found.
		INFOWARE_API_LINKAGE const char* identify_vendor(std::uint64_t vendor_id) noexcept;
	}  // namespace pci
}  // namespace iware
