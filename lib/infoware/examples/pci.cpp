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


#include "infoware/pci.hpp"
#include "infoware/version.hpp"
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>


namespace {
	struct format_pci_id {
		std::uint64_t pci_id;
	};

	std::ostream& operator<<(std::ostream& out, const format_pci_id& id) {
		out << "0x" << std::setw(sizeof(std::uint64_t) * 2) << std::setfill('0') << std::internal << std::hex << std::uppercase << id.pci_id;
		return out;
	}
}  // anonymous namespace

static void print_named_id(const char* subsystem, std::uint64_t pci_id);
static bool print_vendor(std::uint64_t vendor_id, const char* vendor_name);


int main(int, const char** argv) {
	std::cout << "Infoware version " << iware::version << '\n';

	if(argv[1] == nullptr)
		std::cout << "Usage: " << argv[0] << " <vendor_id> [device_id]" << '\n';
	else {
		const auto vendor_id = std::strtoull(argv[1], nullptr, 0);

		if(argv[2] == nullptr) {
			if(!print_vendor(vendor_id, iware::pci::identify_vendor(vendor_id)))
				return 1;
		} else {
			const auto device_id = std::strtoull(argv[2], nullptr, 0);

			const auto device    = iware::pci::identify_device(vendor_id, device_id);
			const auto vendor_ok = print_vendor(vendor_id, device.vendor_name);

			if(device.device_name) {
				std::cout << "Device " << format_pci_id{device_id} << ' ' << device.device_name << '\n';
			} else
				print_named_id("device", device_id);

			if(!vendor_ok || !device.device_name)
				return (vendor_ok ? 0 : 1) | (device.device_name ? 0 : 2);
		}
	}
}


static void print_named_id(const char* subsystem, std::uint64_t pci_id) {
	std::cout << "Unrecognised " << subsystem << " with ID " << format_pci_id{pci_id} << '\n';
}

static bool print_vendor(std::uint64_t vendor_id, const char* vendor_name) {
	if(vendor_name)
		std::cout << "Vendor " << format_pci_id{vendor_id} << ' ' << vendor_name << '\n';
	else
		print_named_id("vendor", vendor_id);

	return vendor_name;
}
