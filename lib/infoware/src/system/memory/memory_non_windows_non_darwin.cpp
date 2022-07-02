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


#include "infoware/system.hpp"
#include <fstream>
#include <string>


iware::system::memory_t iware::system::memory() noexcept {
	std::ifstream meminfo("/proc/meminfo");

	if(!meminfo.is_open() || !meminfo)
		return {};

	iware::system::memory_t ret;
	for(std::string line; std::getline(meminfo, line);) {
		const auto colon_id = line.find_first_of(':');
		const auto value    = std::strtoul(line.c_str() + colon_id + 1, nullptr, 10) * 1024;

		if(line.find("MemTotal") == 0)
			ret.physical_total = value;
		else if(line.find("MemAvailable") == 0)
			ret.physical_available = value;
		else if(line.find("VmallocTotal") == 0)
			ret.virtual_total = value;
		else if(line.find("VmallocUsed") == 0)
			ret.virtual_available = ret.virtual_total - value;
	}

	return ret;
}


#endif
#endif
