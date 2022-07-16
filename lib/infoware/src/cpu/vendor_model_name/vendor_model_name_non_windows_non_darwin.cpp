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


static std::string cpuinfo_value(const char* key) {
	std::ifstream cpuinfo("/proc/cpuinfo");

	if(!cpuinfo.is_open() || !cpuinfo)
		return {};

	for(std::string line; std::getline(cpuinfo, line);)
		if(line.find(key) == 0) {
			const auto colon_id    = line.find_first_of(':');
			const auto nonspace_id = line.find_first_not_of(" \t", colon_id + 1);
			return line.c_str() + nonspace_id;
		}

	return {};
}


std::string iware::cpu::vendor() {
	return cpuinfo_value("vendor");
}

std::string iware::cpu::model_name() {
	return cpuinfo_value("model name");
}


#endif
#endif
