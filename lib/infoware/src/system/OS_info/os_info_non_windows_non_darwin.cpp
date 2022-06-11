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
#include <cstdlib>
#include <fstream>


static iware::system::OS_info_t lsb_release(std::ifstream& release) {
	iware::system::OS_info_t ret{};

	for(std::string line; std::getline(release, line);)
		if(line.find("DISTRIB_ID") == 0)
			ret.name = line.substr(line.find('=') + 1);
		else if(line.find("DISTRIB_RELEASE") == 0) {
			char* marker     = &line[line.find('=') + 1];
			ret.major        = std::strtoul(marker, &marker, 10);
			ret.minor        = std::strtoul(marker + 1, &marker, 10);
			ret.patch        = std::strtoul(marker + 1, &marker, 10);
			ret.build_number = std::strtoul(marker + 1, nullptr, 10);
		} else if(line.find("DISTRIB_DESCRIPTION") == 0) {
			const auto start_idx = line.find('"') + 1;
			const auto end_idx   = line.size() - 1;
			ret.full_name        = line.substr(start_idx, end_idx - start_idx);
		}

	return ret;
}

static void trim_quotes(std::string& in) {
	if(in.empty())
		return;

	if(in.back() == '"')
		in.pop_back();

	if(in.front() == '"')
		in.erase(in.begin());
}


// https://www.linux.org/docs/man5/os-release.html
iware::system::OS_info_t iware::system::OS_info() {
	std::ifstream release("/etc/os-release");

	if(!release.is_open() || !release) {
		release.open("/usr/lib/os-release", std::ios::in);
		if(!release.is_open() || !release) {
			release.open("/etc/lsb-release", std::ios::in);
			if(!release.is_open() || !release)
				return {};
			else
				return lsb_release(release);
		}
	}

	iware::system::OS_info_t ret{};

	for(std::string line; std::getline(release, line);)
		if(line.find("NAME") == 0)
			ret.name = line.substr(line.find('=') + 1);
		else if(line.find("PRETTY_NAME") == 0)
			ret.full_name = line.substr(line.find('=') + 1);
		else if(line.find("VERSION_ID") == 0) {
			char* marker = &line[line.find('=') + 1];
			if(marker[0] == '"')
				++marker;
			ret.major = std::strtoul(marker, &marker, 10);
			if(marker[0] && marker[0] != '"')
				ret.minor = std::strtoul(marker + 1, &marker, 10);
			if(marker[0] && marker[0] != '"')
				ret.patch = std::strtoul(marker + 1, &marker, 10);
			if(marker[0] && marker[0] != '"')
				ret.build_number = std::strtoul(marker + 1, nullptr, 10);
		}

	trim_quotes(ret.name);
	trim_quotes(ret.full_name);

	return ret;
}


#endif
#endif
