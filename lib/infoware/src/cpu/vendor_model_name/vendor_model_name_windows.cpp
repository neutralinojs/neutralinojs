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


template <unsigned int IdentLen>
static std::string central_processor_subkey(const char* key) {
	HKEY hkey;
	if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, R"(HARDWARE\DESCRIPTION\System\CentralProcessor\0)", 0, KEY_READ, &hkey))
		return {};

	char identifier[IdentLen];
	DWORD identifier_len = sizeof(identifier);
	LPBYTE lpdata        = static_cast<LPBYTE>(static_cast<void*>(&identifier[0]));
	if(RegQueryValueExA(hkey, key, nullptr, nullptr, lpdata, &identifier_len))
		return {};

	return identifier;
}

std::string iware::cpu::vendor() {
	return central_processor_subkey<12 + 1>("VendorIdentifier");
}

std::string iware::cpu::model_name() {
	return central_processor_subkey<64 + 1>("ProcessorNameString");
}


#endif
