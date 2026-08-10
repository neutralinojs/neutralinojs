#if !defined(_WIN32) && !defined(__linux__) && !defined(__APPLE__)


#include "infoware/system.hpp"


std::vector<iware::system::connected_device_t> iware::system::connected_devices() {
	return {};
}


#endif
