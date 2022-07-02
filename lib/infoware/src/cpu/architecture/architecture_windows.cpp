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


#include "infoware/cpu.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724958(v=vs.85).aspx
iware::cpu::architecture_t iware::cpu::architecture() noexcept {
	SYSTEM_INFO sysinfo;
	GetNativeSystemInfo(&sysinfo);

	switch(sysinfo.wProcessorArchitecture) {
		case PROCESSOR_ARCHITECTURE_AMD64:
			return iware::cpu::architecture_t::x64;
		case PROCESSOR_ARCHITECTURE_ARM:
			return iware::cpu::architecture_t::arm;
		case PROCESSOR_ARCHITECTURE_IA64:
			return iware::cpu::architecture_t::itanium;
		case PROCESSOR_ARCHITECTURE_INTEL:
			return iware::cpu::architecture_t::x86;
		default:
			return iware::cpu::architecture_t::unknown;
	}
}


#endif
