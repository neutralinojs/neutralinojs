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


#endif
