#ifndef _WIN32

#include "infoware/system.hpp"
#include <cerrno>
#include <cstdint>
#include <fstream>
#include <limits.h>
#include <string>
#include <sys/statvfs.h>
#include <unistd.h>
#include <vector>

#if defined(__APPLE__) || defined(__FreeBSD__)
#include <sys/mount.h>
#endif

using namespace std;
using std::uint64_t;


namespace {
	struct mount_info_t {
		string name;
		string mount_point;
		string file_system;
	};

	static string current_path() {
		vector<char> buffer(PATH_MAX);
		while(true) {
			if(getcwd(buffer.data(), buffer.size()))
				return buffer.data();
			if(errno != ERANGE)
				return {};
			buffer.resize(buffer.size() * 2);
		}
	}

	static bool is_path_within_mount(const string& path, const string& mount_point) {
		if(mount_point == "/")
			return !path.empty() && path[0] == '/';
		if(path.size() < mount_point.size())
			return false;
		if(path.compare(0, mount_point.size(), mount_point) != 0)
			return false;
		return path.size() == mount_point.size() || path[mount_point.size()] == '/';
	}

	static string unescape_mount_field(const string& field) {
		string ret;
		for(size_t i = 0; i < field.size(); ++i) {
			if(field[i] == '\\' && i + 3 < field.size() && field[i + 1] >= '0' && field[i + 1] <= '7' &&
			   field[i + 2] >= '0' && field[i + 2] <= '7' && field[i + 3] >= '0' && field[i + 3] <= '7') {
				const auto value = ((field[i + 1] - '0') << 6) + ((field[i + 2] - '0') << 3) + (field[i + 3] - '0');
				ret.push_back(static_cast<char>(value));
				i += 3;
			}
			else {
				ret.push_back(field[i]);
			}
		}
		return ret;
	}

#if defined(__linux__)
	static mount_info_t find_mount_info(const string& path) {
		ifstream mounts("/proc/self/mounts");
		if(!mounts.is_open())
			mounts.open("/proc/mounts");

		mount_info_t best{};
		string device;
		string mount_point;
		string file_system;
		string options;
		while(mounts >> device >> mount_point >> file_system >> options) {
			mount_point = unescape_mount_field(mount_point);
			if(is_path_within_mount(path, mount_point) && mount_point.size() >= best.mount_point.size()) {
				best.name        = unescape_mount_field(device);
				best.mount_point = mount_point;
				best.file_system = unescape_mount_field(file_system);
			}

			string ignored;
			getline(mounts, ignored);
		}
		return best;
	}
#elif defined(__APPLE__) || defined(__FreeBSD__)
	static mount_info_t find_mount_info(const string& path) {
		struct statfs* mounts = nullptr;
		const int count       = getmntinfo(&mounts, MNT_NOWAIT);
		mount_info_t best{};

		for(int i = 0; i < count; ++i) {
			const string mount_point = mounts[i].f_mntonname;
			if(is_path_within_mount(path, mount_point) && mount_point.size() >= best.mount_point.size()) {
				best.name        = mounts[i].f_mntfromname;
				best.mount_point = mount_point;
				best.file_system = mounts[i].f_fstypename;
			}
		}
		return best;
	}
#else
	static mount_info_t find_mount_info(const string&) {
		return {};
	}
#endif
}

iware::system::disk_info_t iware::system::disk_info() {
	const auto path = current_path();
	if(path.empty())
		return {};

	struct statvfs stats {};
	if(statvfs(path.c_str(), &stats) != 0)
		return {};

	auto mount_info = find_mount_info(path);
	if(mount_info.mount_point.empty())
		mount_info.mount_point = path;

	const auto block_size = static_cast<uint64_t>(stats.f_frsize ? stats.f_frsize : stats.f_bsize);
	iware::system::disk_info_t ret{};
	ret.name         = mount_info.name;
	ret.mount_point  = mount_info.mount_point;
	ret.file_system  = mount_info.file_system;
	ret.total        = static_cast<uint64_t>(stats.f_blocks) * block_size;
	ret.free         = static_cast<uint64_t>(stats.f_bavail) * block_size;
	ret.used         = ret.total >= ret.free ? ret.total - ret.free : 0;
	ret.used_percent = ret.total == 0 ? 0.0 : (static_cast<double>(ret.used) * 100.0) / static_cast<double>(ret.total);
	return ret;
}


#endif
