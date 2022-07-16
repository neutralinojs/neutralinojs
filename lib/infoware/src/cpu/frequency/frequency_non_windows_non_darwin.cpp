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
#ifndef __APPLE__


#include "infoware/cpu.hpp"
#include <fstream>
#include <string>


std::uint64_t iware::cpu::frequency() noexcept {
	std::ifstream cpuinfo("/proc/cpuinfo");

	if(!cpuinfo.is_open() || !cpuinfo)
		return 0;

	for(std::string line; std::getline(cpuinfo, line);)
		if(line.find("cpu MHz") == 0) {
			const auto colon_id = line.find_first_of(':');
			return static_cast<std::uint64_t>(std::strtod(line.c_str() + colon_id + 1, nullptr) * 1'000'000);
		}

	return 0;
}


#endif
#endif
