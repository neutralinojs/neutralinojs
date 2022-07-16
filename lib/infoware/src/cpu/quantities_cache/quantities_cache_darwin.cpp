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


#include "infoware/cpu.hpp"
#include "infoware/detail/sysctl.hpp"


// https://github.com/ThePhD/infoware/issues/12#issuecomment-495291650
iware::cpu::quantities_t iware::cpu::quantities() {
	iware::cpu::quantities_t ret{};

	const auto ctl_thread_data = iware::detail::sysctl("machdep.cpu.thread_count");
	if(!ctl_thread_data.empty()) {
		const auto thread_data = iware::detail::deconstruct_sysctl_int(ctl_thread_data);
		if(thread_data.first)
			ret.logical = thread_data.second;
	}

	const auto ctl_core_data = iware::detail::sysctl("machdep.cpu.core_count");
	if(!ctl_core_data.empty()) {
		const auto core_data = iware::detail::deconstruct_sysctl_int(ctl_core_data);
		if(core_data.first)
			ret.physical = core_data.second;
	}

	const auto ctl_packages_data = iware::detail::sysctl("hw.packages");
	if(!ctl_packages_data.empty()) {
		const auto packages_data = iware::detail::deconstruct_sysctl_int(ctl_packages_data);
		if(packages_data.first)
			ret.packages = packages_data.second;
	}

	return ret;
}

// This is hell
//
// https://github.com/ThePhD/infoware/issues/12#issuecomment-495782115
//
// TODO: couldn't find a good way to get the associativity (default 0) or the type (default unified)
iware::cpu::cache_t iware::cpu::cache(unsigned int level) {
	// Unspecified keys default to nullptr
	static const char* size_keys[][3]{{}, {"hw.l1icachesize", "hw.l1dcachesize", "hw.l1cachesize"}, {"hw.l2cachesize"}, {"hw.l3cachesize"}};


	iware::cpu::cache_t ret{};

	const auto ctl_cachelinesize_data = iware::detail::sysctl("hw.cachelinesize");
	if(!ctl_cachelinesize_data.empty()) {
		const auto cachelinesize_data = iware::detail::deconstruct_sysctl_int(ctl_cachelinesize_data);
		if(cachelinesize_data.first)
			ret.line_size = cachelinesize_data.second;
	}

	if(level < sizeof(size_keys) / sizeof(*size_keys))
		for(auto key : size_keys[level]) {
			if(!key)
				break;

			const auto ctl_cachesize_data = iware::detail::sysctl(key);
			if(!ctl_cachesize_data.empty()) {
				const auto cachesize_data = iware::detail::deconstruct_sysctl_int(ctl_cachesize_data);
				if(cachesize_data.first)
					ret.size += cachesize_data.second;
			}
		}

	return ret;
}


#endif
