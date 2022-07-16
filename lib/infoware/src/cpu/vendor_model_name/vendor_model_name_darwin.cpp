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
#include <string>


using namespace std::literals;


static std::string sysctl_value(const char* subkey) {
	auto ctl_data = iware::detail::sysctl(("machdep.cpu."s + subkey).c_str());
	if(ctl_data.empty())
		return {};
	else
		return ctl_data.data();
}


std::string iware::cpu::vendor() {
	return sysctl_value("vendor");
}

std::string iware::cpu::model_name() {
	return sysctl_value("brand_string");
}


#endif
