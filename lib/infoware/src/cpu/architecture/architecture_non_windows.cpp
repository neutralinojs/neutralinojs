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


#include "infoware/cpu.hpp"
#include <cstring>
#include <sys/utsname.h>


// https://github.com/karelzak/util-linux/blob/master/sys-utils/lscpu.c
iware::cpu::architecture_t iware::cpu::architecture() noexcept {
	utsname buf;

	if(uname(&buf) == -1)
		return iware::cpu::architecture_t::unknown;

	if(!strcmp(buf.machine, "x86_64"))
		return iware::cpu::architecture_t::x64;
	else if(strstr(buf.machine, "arm") == buf.machine)
		return iware::cpu::architecture_t::arm;
	else if(!strcmp(buf.machine, "ia64") || !strcmp(buf.machine, "IA64"))
		return iware::cpu::architecture_t::itanium;
	else if(!strcmp(buf.machine, "i686"))
		return iware::cpu::architecture_t::x86;
	else
		return iware::cpu::architecture_t::unknown;
}


#endif
