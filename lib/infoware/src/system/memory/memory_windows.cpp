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


#ifdef _WIN32


#include "infoware/system.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


iware::system::memory_t iware::system::memory() noexcept {
	MEMORYSTATUSEX mem;
	mem.dwLength = sizeof(mem);
	if(!GlobalMemoryStatusEx(&mem))
		return {};

	return {mem.ullAvailPhys, mem.ullTotalPhys, mem.ullAvailVirtual, mem.ullTotalVirtual};
}


#endif
