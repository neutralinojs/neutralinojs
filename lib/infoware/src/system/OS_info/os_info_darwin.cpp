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


#include "infoware/detail/sysctl.hpp"
#include "infoware/system.hpp"
#include <cstdlib>


iware::system::OS_info_t iware::system::OS_info() {
	auto ctl_ostype_data  = iware::detail::sysctl("kern.ostype");
	auto ctl_version_data = iware::detail::sysctl("kern.version");

	auto ctl_osrelease_data = iware::detail::sysctl("kern.osrelease");
	char buf{};
	char* osrelease_marker = ctl_osrelease_data.empty() ? &buf : ctl_osrelease_data.data();

	const auto ctl_osrevision_data = iware::detail::sysctl("kern.osrevision");
	unsigned int build_number{};
	if(!ctl_osrevision_data.empty()) {
		const auto osrevision_data = iware::detail::deconstruct_sysctl_int(ctl_osrevision_data);
		if(osrevision_data.first)
			build_number = osrevision_data.second;
	}

	return {ctl_ostype_data.empty() ? "" : ctl_ostype_data.data(),                                 //
	        ctl_version_data.empty() ? "" : ctl_version_data.data(),                               //
	        static_cast<unsigned int>(std::strtoul(osrelease_marker, &osrelease_marker, 10)),      //
	        static_cast<unsigned int>(std::strtoul(osrelease_marker + 1, &osrelease_marker, 10)),  //
	        static_cast<unsigned int>(std::strtoul(osrelease_marker + 1, nullptr, 10)),            //
	        build_number};
}


#endif
