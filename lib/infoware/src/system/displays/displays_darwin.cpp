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
//
//
// Darwin support via Carbon API originally by Arthur Biancarelli – https://github.com/ThePhD/infoware/issues/17#issuecomment-538789869


#ifdef __APPLE__


#include "infoware/detail/scope.hpp"
#include "infoware/system.hpp"
#include <ApplicationServices/ApplicationServices.h>
#include <algorithm>
#include <cstring>


template <class T, class F>
static std::vector<T> enumerate_displays(F&& cbk) {
	std::uint32_t num_displays;
	if(CGGetActiveDisplayList(0, nullptr, &num_displays) != kCGErrorSuccess)
		return {};

	std::vector<CGDirectDisplayID> display_ids(num_displays);
	if(CGGetActiveDisplayList(num_displays, display_ids.data(), &num_displays) != kCGErrorSuccess)
		return {};


	std::vector<T> ret;
	ret.reserve(num_displays);

	for(std::size_t i = 0; i < num_displays; ++i)
		ret.emplace_back(std::move(cbk(display_ids[i])));

	return ret;
}


std::vector<iware::system::display_t> iware::system::displays() {
	return enumerate_displays<iware::system::display_t>([](auto display_id) {
		const std::uint32_t width = CGDisplayPixelsWide(display_id);
		// 25.4 millimeters per inch
		const std::uint32_t dpi = 25.4 * CGDisplayScreenSize(display_id).width / width;

		auto display_mode = CGDisplayCopyDisplayMode(display_id);
		iware::detail::quickscope_wrapper display_mode_deleter{[&]() { CGDisplayModeRelease(display_mode); }};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
		// A string in the form --------RRRRRRRRGGGGGGGGBBBBBBBB
		auto pixel_encoding_raw = CGDisplayModeCopyPixelEncoding(display_mode);
		iware::detail::quickscope_wrapper pixel_encoding_raw_deleter{[&]() { CFRelease(pixel_encoding_raw); }};
#pragma GCC diagnostic pop

		// Count the number of occurences of R/G/B pixels
		std::uint32_t bpp{};
		if(const auto pixel_encoding = CFStringGetCStringPtr(pixel_encoding_raw, kCFStringEncodingUTF8))
			bpp = std::count_if(pixel_encoding, pixel_encoding + std::strlen(pixel_encoding), [](auto c) { return c == 'R' || c == 'G' || c == 'B'; });


		return iware::system::display_t{width, static_cast<std::uint32_t>(CGDisplayPixelsHigh(display_id)), dpi, bpp,
		                                // The refresh rate, in hertz, of the specified display mode for a CRT display.
		                                // Some displays may not use conventional video vertical and horizontal sweep in painting the screen;
		                                // for these displays, the return value is 0.
		                                //
		                                // Tested to return 0 for Retina displays
		                                CGDisplayModeGetRefreshRate(display_mode)};
	});
}

std::vector<std::vector<iware::system::display_config_t>> iware::system::available_display_configurations() {
	return enumerate_displays<std::vector<iware::system::display_config_t>>([](auto display_id) {
		auto modes = CGDisplayCopyAllDisplayModes(display_id, nullptr);
		if(!modes)
			return std::vector<iware::system::display_config_t>{};
		iware::detail::quickscope_wrapper modes_deleter{[&]() { CFRelease(modes); }};

		const auto modes_len = CFArrayGetCount(modes);
		std::vector<iware::system::display_config_t> ret;
		ret.reserve(modes_len);

		for(auto i = 0l; i < modes_len; ++i) {
			auto mode = static_cast<CGDisplayModeRef>(const_cast<void*>(CFArrayGetValueAtIndex(modes, i)));

			ret.emplace_back(iware::system::display_config_t{static_cast<std::uint32_t>(CGDisplayModeGetWidth(mode)),
			                                                 static_cast<std::uint32_t>(CGDisplayModeGetHeight(mode)),
			                                                 {CGDisplayModeGetRefreshRate(mode)}});
		}

		return ret;
	});
}


#endif
