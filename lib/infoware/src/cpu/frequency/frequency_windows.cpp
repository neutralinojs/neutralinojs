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


std::uint64_t iware::cpu::frequency() noexcept {
	HKEY hkey;
	if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, R"(HARDWARE\DESCRIPTION\System\CentralProcessor\0)", 0, KEY_READ, &hkey)) {
		// Fallback
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		return freq.QuadPart * 1'000;
	}

	DWORD freq_mhz;
	DWORD freq_mhz_len = sizeof(freq_mhz);
	if(RegQueryValueExA(hkey, "~MHz", nullptr, nullptr, static_cast<LPBYTE>(static_cast<void*>(&freq_mhz)), &freq_mhz_len))
		return {};

	return freq_mhz * 1'000'000;
}


#endif
