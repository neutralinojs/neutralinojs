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


std::uint64_t iware::cpu::frequency() noexcept {
	const auto ctl_data = iware::detail::sysctl("hw.cpufrequency");
	if(ctl_data.empty())
		return 0;

	const auto data = iware::detail::deconstruct_sysctl_int(ctl_data);
	if(!data.first)
		return 0;

	return data.second;
}


#endif
