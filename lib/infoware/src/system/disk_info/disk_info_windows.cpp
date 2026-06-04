#ifdef _WIN32

#include "infoware/detail/winstring.hpp"
#include "infoware/system.hpp"
#include <string>
#include <vector>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using namespace std;


static string trim_trailing_separators(string path) {
	while(path.size() > 1 && (path.back() == '\\' || path.back() == '/'))
		path.pop_back();
	return path;
}

iware::system::disk_info_t iware::system::disk_info() {
	vector<wchar_t> current_dir(GetCurrentDirectoryW(0, nullptr));
	if(current_dir.empty())
		return {};
	if(!GetCurrentDirectoryW(static_cast<DWORD>(current_dir.size()), current_dir.data()))
		return {};

	vector<wchar_t> volume_path(MAX_PATH + 1);
	if(!GetVolumePathNameW(current_dir.data(), volume_path.data(), static_cast<DWORD>(volume_path.size())))
		return {};

	ULARGE_INTEGER free_bytes_available;
	ULARGE_INTEGER total_bytes;
	if(!GetDiskFreeSpaceExW(volume_path.data(), &free_bytes_available, &total_bytes, nullptr))
		return {};

	vector<wchar_t> file_system(MAX_PATH + 1);
	GetVolumeInformationW(volume_path.data(), nullptr, 0, nullptr, nullptr, nullptr, file_system.data(), static_cast<DWORD>(file_system.size()));

	iware::system::disk_info_t ret{};
	ret.mount_point  = iware::detail::narrowen_winstring(volume_path.data());
	ret.name         = trim_trailing_separators(ret.mount_point);
	ret.file_system  = iware::detail::narrowen_winstring(file_system.data());
	ret.total        = total_bytes.QuadPart;
	ret.free         = free_bytes_available.QuadPart;
	ret.used         = ret.total >= ret.free ? ret.total - ret.free : 0;
	ret.used_percent = ret.total == 0 ? 0.0 : (static_cast<double>(ret.used) * 100.0) / static_cast<double>(ret.total);
	return ret;
}


#endif
