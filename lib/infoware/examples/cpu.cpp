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


#include "infoware/cpu.hpp"
#include "infoware/version.hpp"
#include <iostream>
#include <utility>


static const char* cache_type_name(iware::cpu::cache_type_t cache_type) noexcept;
static const char* architecture_name(iware::cpu::architecture_t architecture) noexcept;
static const char* endianness_name(iware::cpu::endianness_t endianness) noexcept;


int main() {
	std::cout << "Infoware version " << iware::version << '\n';

	{
		const auto quantities = iware::cpu::quantities();
		std::cout << "\n"
		             "  Quantities:\n"
		          << "    Logical CPUs : " << quantities.logical << '\n'
		          << "    Physical CPUs: " << quantities.physical << '\n'
		          << "    CPU packages : " << quantities.packages << '\n';
	}

	{
		std::cout << "\n"
		             "  Caches:\n";
		for(auto i = 1u; i <= 3; ++i) {
			const auto cache = iware::cpu::cache(i);
			std::cout << "    L" << i << ":\n"
			          << "      Size         : " << cache.size << "B\n"
			          << "      Line size    : " << cache.line_size << "B\n"
			          << "      Associativity: " << static_cast<unsigned int>(cache.associativity) << '\n'
			          << "      Type         : " << cache_type_name(cache.type) << '\n';
		}
	}

	{
		std::cout << "\n"
		          << "  Architecture: " << architecture_name(iware::cpu::architecture()) << '\n'
		          << "  Frequency: " << iware::cpu::frequency() << " Hz\n"
		          << "  Endianness: " << endianness_name(iware::cpu::endianness()) << '\n'
		          << "  Model name: " << iware::cpu::model_name() << '\n'
		          << "  Vendor ID: " << iware::cpu::vendor_id() << '\n';
	}

	{
		std::cout << std::boolalpha
		          << "\n"
		             "  Instruction set support:\n";
		for(auto&& set : {std::make_pair("3D-now!", iware::cpu::instruction_set_t::s3d_now),  //
		                  std::make_pair("MMX    ", iware::cpu::instruction_set_t::mmx),      //
		                  std::make_pair("SSE    ", iware::cpu::instruction_set_t::sse),      //
		                  std::make_pair("SSE2   ", iware::cpu::instruction_set_t::sse2),     //
		                  std::make_pair("SSE3   ", iware::cpu::instruction_set_t::sse3),     //
		                  std::make_pair("AVX    ", iware::cpu::instruction_set_t::avx)})
			std::cout << "    " << set.first << ": " << iware::cpu::instruction_set_supported(set.second) << '\n';
	}

	std::cout << '\n';
}


static const char* cache_type_name(iware::cpu::cache_type_t cache_type) noexcept {
	switch(cache_type) {
		case iware::cpu::cache_type_t::unified:
			return "Unified";
		case iware::cpu::cache_type_t::instruction:
			return "Instruction";
		case iware::cpu::cache_type_t::data:
			return "Data";
		case iware::cpu::cache_type_t::trace:
			return "Trace";
		default:
			return "Unknown";
	}
}

static const char* architecture_name(iware::cpu::architecture_t architecture) noexcept {
	switch(architecture) {
		case iware::cpu::architecture_t::x64:
			return "x64";
		case iware::cpu::architecture_t::arm:
			return "ARM";
		case iware::cpu::architecture_t::itanium:
			return "Itanium";
		case iware::cpu::architecture_t::x86:
			return "x86";
		default:
			return "Unknown";
	}
}

static const char* endianness_name(iware::cpu::endianness_t endianness) noexcept {
	switch(endianness) {
		case iware::cpu::endianness_t::little:
			return "Little-Endian";
		case iware::cpu::endianness_t::big:
			return "Big-Endian";
		default:
			return "Unknown";
	}
}
