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


#pragma once


#include <infoware/linkage.hpp>

#include <cstdint>
#include <utility>
#include <vector>


namespace iware {
	namespace detail {
		/// https://github.com/ThePhD/infoware/issues/13
		INFOWARE_API_LINKAGE_INTERNAL std::vector<char> sysctl(const char* name);
		INFOWARE_API_LINKAGE_INTERNAL std::vector<char> sysctl(int mib_0, int mib_1);

		INFOWARE_API_LINKAGE_INTERNAL std::pair<bool, std::uint64_t> deconstruct_sysctl_int(const std::vector<char>& data);
	}  // namespace detail
}  // namespace iware
