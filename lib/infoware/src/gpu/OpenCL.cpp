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


#ifndef INFOWARE_USE_D3D
#ifdef INFOWARE_USE_OPENCL

#include "infoware/gpu.hpp"
#include <cstdlib>
#include <cstring>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif


static iware::gpu::vendor_t parse_vendor(const char* name) {
	if(!std::strcmp(name, "Intel(R) Corporation") || !std::strcmp(name, "Intel"))
		return iware::gpu::vendor_t::intel;
	else if(!std::strcmp(name, "Advanced Micro Devices, Inc.") || !std::strcmp(name, "AMD"))
		return iware::gpu::vendor_t::amd;
	else if(!std::strcmp(name, "NVIDIA Corporation"))
		return iware::gpu::vendor_t::nvidia;
	else if(!std::strcmp(name, "Apple"))
		return iware::gpu::vendor_t::apple;
	else
		return iware::gpu::vendor_t::unknown;
}


/// See https://github.com/Oblomov/clinfo/blob/bfc628e74d4cdbf5c5adca169e8bae540fca92ef/src/clinfo.c#L2104 for other available props
std::vector<iware::gpu::device_properties_t> iware::gpu::device_properties() {
	cl_platform_id platforms[64];
	cl_uint platforms_used;
	clGetPlatformIDs(sizeof(platforms) / sizeof(*platforms), platforms, &platforms_used);

	std::vector<iware::gpu::device_properties_t> ret;
	for(auto i = 0u; i < platforms_used; ++i) {
		cl_device_id devices[64];
		cl_uint devices_used;
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, sizeof(devices) / sizeof(*devices), devices, &devices_used);

		for(auto j = 0u; j < devices_used; ++j) {
			char name[256];
			char vendorname[256];
			cl_ulong cache;
			cl_ulong memory;
			cl_uint max_frequency;

			clGetDeviceInfo(devices[j], CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(cache), &cache, nullptr);
			clGetDeviceInfo(devices[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(max_frequency), &max_frequency, nullptr);
			clGetDeviceInfo(devices[j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(memory), &memory, nullptr);
			clGetDeviceInfo(devices[j], CL_DEVICE_VENDOR, sizeof(vendorname) / sizeof(*vendorname), &vendorname, nullptr);
			clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(name) / sizeof(*name), &name, nullptr);

			ret.push_back({parse_vendor(vendorname), name, memory, cache, max_frequency * 1'000'000});
		}
	}

	return ret;
}


#endif
#endif
