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
#ifndef INFOWARE_USE_X11


#include "infoware/system.hpp"


std::vector<iware::system::display_t> iware::system::displays() {
	return {};
}

std::vector<std::vector<iware::system::display_config_t>> iware::system::available_display_configurations() {
	return {};
}


#endif
#endif
#endif
