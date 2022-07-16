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
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <string>
#include <unistd.h>
#include <vector>


// http://stackoverflow.com/a/150971/2851815
// https://github.com/karelzak/util-linux/blob/25b7045e5db032df5354c0749cb454a20b89c522/sys-utils/lscpu.c#L1119
iware::cpu::quantities_t iware::cpu::quantities() {
	iware::cpu::quantities_t ret{};
	ret.logical = sysconf(_SC_NPROCESSORS_ONLN);

	std::ifstream cpuinfo("/proc/cpuinfo");

	if(!cpuinfo.is_open() || !cpuinfo)
		return ret;

	std::vector<unsigned int> package_ids;
	for(std::string line; std::getline(cpuinfo, line);)
		if(line.find("physical id") == 0) {
			const auto physical_id = std::strtoul(line.c_str() + line.find_first_of("1234567890"), nullptr, 10);
			if(std::find(package_ids.begin(), package_ids.end(), physical_id) == package_ids.end())
				package_ids.emplace_back(physical_id);
		}

	ret.packages = package_ids.size();
	ret.physical = ret.logical / ret.packages;

	return ret;
}

// http://superuser.com/a/203481
iware::cpu::cache_t iware::cpu::cache(unsigned int level) {
	std::string prefix("/sys/devices/system/cpu/cpu0/cache/index" + std::to_string(level) + '/');
	iware::cpu::cache_t ret{};

	{
		std::ifstream size(prefix + "size");
		if(size.is_open() && size) {
			char suffix;
			size >> ret.size >> suffix;
			switch(suffix) {
				case 'G':
					ret.size *= 1024;
					[[fallthrough]];
				case 'M':
					ret.size *= 1024;
					[[fallthrough]];
				case 'K':
					ret.size *= 1024;
			}
		}
	}

	{
		std::ifstream line_size(prefix + "coherency_line_size");
		if(line_size.is_open() && line_size)
			line_size >> ret.line_size;
	}

	{
		std::ifstream associativity(prefix + "associativity");
		if(associativity.is_open() && associativity) {
			unsigned int tmp;
			associativity >> tmp;
			ret.associativity = tmp;
		}
	}

	{
		std::ifstream type(prefix + "type");
		if(type.is_open() && type) {
			std::string tmp;
			type >> tmp;
			if(tmp.find("nified") == 1)
				ret.type = iware::cpu::cache_type_t::unified;
			else if(tmp.find("nstruction") == 1)
				ret.type = iware::cpu::cache_type_t::instruction;
			else if(tmp.find("ata") == 1)
				ret.type = iware::cpu::cache_type_t::data;
			else if(tmp.find("race") == 1)
				ret.type = iware::cpu::cache_type_t::trace;
		}
	}

	return ret;
}


#endif
#endif
