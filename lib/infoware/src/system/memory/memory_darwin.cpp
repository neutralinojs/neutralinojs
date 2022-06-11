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


#include "infoware/detail/sysctl.hpp"
#include "infoware/system.hpp"
#include <cstdint>
#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>


/// Adapted from https://stackoverflow.com/q/14789672/2851815
iware::system::memory_t iware::system::memory() noexcept {
	iware::system::memory_t ret{};

	const auto host = mach_host_self();

	const auto ctl_ram = iware::detail::sysctl(CTL_HW, HW_MEMSIZE);
	if(!ctl_ram.empty()) {
		const auto ram = iware::detail::deconstruct_sysctl_int(ctl_ram);
		if(ram.first)
			ret.virtual_total = ram.second;
	}

	vm_statistics64 stats;
	natural_t count = HOST_VM_INFO64_COUNT;
	if(host_statistics64(host, HOST_VM_INFO64, reinterpret_cast<host_info64_t>(&stats), &count) == KERN_SUCCESS)
		ret.virtual_available = stats.free_count * getpagesize();

	ret.physical_total     = ret.virtual_total;
	ret.physical_available = ret.virtual_available;

	return ret;
}


#endif
