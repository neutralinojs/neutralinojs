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

#if defined _MSC_VER
	#if defined INFOWARE_DLL
		#if defined INFOWARE_BUILDING
			#define INFOWARE_API_LINKAGE __declspec(dllexport)
			#define INFOWARE_API_LINKAGE_INTERNAL
		#else
			#define INFOWARE_API_LINKAGE __declspec(dllimport)
			#define INFOWARE_API_LINKAGE_INTERNAL
		#endif // FICAPI_BUILD - Building the Library vs. Using the Library
	#else
		#define INFOWARE_API_LINKAGE
		#define INFOWARE_API_LINKAGE_INTERNAL
	#endif // Building a DLL vs. Static Library
#else  // g++ / clang++
	#define INFOWARE_API_LINKAGE __attribute__((visibility("default")))
	#define INFOWARE_API_LINKAGE_INTERNAL __attribute__((visibility("hidden")))
#endif // MSVC vs. other shared object / dll attributes

