#ifdef __linux__


#include "connected_devices.hpp"

#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <system_error>
#include <vector>


namespace {
	std::string read_text_file(const std::filesystem::path& path) {
		std::ifstream file(path);
		if(!file.is_open())
			return "";

		std::string value;
		std::getline(file, value);
		return iware::system::detail::trim(value);
	}

	bool bitmap_has_bit(const std::string& bitmap, unsigned int bit, unsigned int word_bits) {
		if(bitmap.empty() || word_bits == 0)
			return false;

		std::vector<unsigned long long> words;
		std::string token;
		std::stringstream stream(bitmap);
		while(stream >> token) {
			try {
				words.push_back(std::stoull(token, nullptr, 16));
			} catch(...) {
				words.push_back(0);
			}
		}

		const std::size_t word_index = bit / word_bits;
		if(word_index >= words.size())
			return false;

		const std::size_t bitmap_index = words.size() - 1 - word_index;
		const unsigned int bit_index   = bit % word_bits;
		return (words[bitmap_index] & (1ULL << bit_index)) != 0;
	}

	bool bitmap_has_bit(const std::string& bitmap, unsigned int bit) {
		return bitmap_has_bit(bitmap, bit, 64) || bitmap_has_bit(bitmap, bit, 32);
	}

	iware::system::connected_device_type_t input_device_type(const std::filesystem::path& device_path,
	                                                         const std::string& name) {
		using iware::system::connected_device_type_t;

		const std::string lower_name = iware::system::detail::to_lower(name);

		if(lower_name.find("power button") != std::string::npos || lower_name.find("sleep button") != std::string::npos ||
		   lower_name.find("lid switch") != std::string::npos || lower_name.find("video bus") != std::string::npos ||
		   lower_name.find("pc speaker") != std::string::npos)
			return connected_device_type_t::unknown;

		if(lower_name.find("touchpad") != std::string::npos || lower_name.find("trackpad") != std::string::npos)
			return connected_device_type_t::touchpad;
		if(lower_name.find("touchscreen") != std::string::npos || lower_name.find("touch screen") != std::string::npos)
			return connected_device_type_t::touchscreen;
		if(lower_name.find("stylus") != std::string::npos || lower_name.find("wacom") != std::string::npos ||
		   lower_name.find(" pen") != std::string::npos)
			return connected_device_type_t::pen;
		if(lower_name.find("gamepad") != std::string::npos || lower_name.find("game pad") != std::string::npos ||
		   lower_name.find("controller") != std::string::npos)
			return connected_device_type_t::gamepad;
		if(lower_name.find("joystick") != std::string::npos)
			return connected_device_type_t::joystick;
		if(lower_name.find("keyboard") != std::string::npos || lower_name.find("kbd") != std::string::npos)
			return connected_device_type_t::keyboard;
		if(lower_name.find("mouse") != std::string::npos)
			return connected_device_type_t::mouse;

		const std::string ev  = read_text_file(device_path / "capabilities" / "ev");
		const std::string key = read_text_file(device_path / "capabilities" / "key");
		const std::string rel = read_text_file(device_path / "capabilities" / "rel");
		const std::string abs = read_text_file(device_path / "capabilities" / "abs");

		const bool has_ev_key     = bitmap_has_bit(ev, 0x01);
		const bool has_ev_rel     = bitmap_has_bit(ev, 0x02);
		const bool has_ev_abs     = bitmap_has_bit(ev, 0x03);
		const bool has_relative_xy = bitmap_has_bit(rel, 0x00) && bitmap_has_bit(rel, 0x01);
		const bool has_absolute_xy =
		    (bitmap_has_bit(abs, 0x00) && bitmap_has_bit(abs, 0x01)) ||
		    (bitmap_has_bit(abs, 0x35) && bitmap_has_bit(abs, 0x36));

		if(bitmap_has_bit(key, 0x14b) || bitmap_has_bit(key, 0x14c))
			return connected_device_type_t::pen;
		if(bitmap_has_bit(key, 0x130))
			return connected_device_type_t::gamepad;
		if(bitmap_has_bit(key, 0x120))
			return connected_device_type_t::joystick;

		const bool has_mouse_button =
		    bitmap_has_bit(key, 0x110) || bitmap_has_bit(key, 0x111) || bitmap_has_bit(key, 0x112);
		if(has_ev_rel && has_relative_xy && has_mouse_button)
			return connected_device_type_t::mouse;

		const bool has_touch  = bitmap_has_bit(key, 0x14a);
		const bool has_finger = bitmap_has_bit(key, 0x145);
		if(has_ev_abs && has_absolute_xy && has_touch && has_finger)
			return connected_device_type_t::touchpad;
		if(has_ev_abs && has_absolute_xy && has_touch)
			return connected_device_type_t::touchscreen;

		const bool has_keyboard_keys =
		    (bitmap_has_bit(key, 0x1e) && bitmap_has_bit(key, 0x30) && bitmap_has_bit(key, 0x2e)) ||
		    (bitmap_has_bit(key, 0x01) && bitmap_has_bit(key, 0x1c) && bitmap_has_bit(key, 0x39));
		if(has_ev_key && has_keyboard_keys)
			return connected_device_type_t::keyboard;

		if(has_ev_key || has_ev_rel || has_ev_abs)
			return connected_device_type_t::hid;
		return connected_device_type_t::unknown;
	}
}  // namespace


std::vector<iware::system::connected_device_t> iware::system::connected_devices() {
	std::vector<iware::system::connected_device_t> devices;
	std::set<std::string> seen_devices;

	std::error_code error;
	const std::filesystem::path input_path("/sys/class/input");
	if(!std::filesystem::exists(input_path, error) || error)
		return devices;

	std::filesystem::directory_iterator iterator(input_path, error);
	const std::filesystem::directory_iterator end;
	if(error)
		return devices;

	for(; iterator != end; iterator.increment(error)) {
		if(error)
			return devices;

		const std::filesystem::path event_path = iterator->path();
		const std::string filename             = event_path.filename().string();
		if(filename.rfind("event", 0) != 0)
			continue;

		const std::filesystem::path device_path = event_path / "device";
		const std::string name                  = read_text_file(device_path / "name");
		const connected_device_type_t type      = input_device_type(device_path, name);
		const std::string vendor_id             = read_text_file(device_path / "id" / "vendor");
		const std::string product_id            = read_text_file(device_path / "id" / "product");

		detail::add_connected_device(devices, seen_devices, name, type, vendor_id, product_id);
	}

	return devices;
}


#endif
