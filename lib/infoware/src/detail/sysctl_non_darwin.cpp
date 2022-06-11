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


#ifndef __APPLE__


#include "infoware/detail/sysctl.hpp"


std::vector<char> iware::detail::sysctl(const char*) {
	return {};
}

std::vector<char> iware::detail::sysctl(int, int) {
	return {};
}

std::pair<bool, std::uint64_t> iware::detail::deconstruct_sysctl_int(const std::vector<char>&) {
	return {};
}


#endif
