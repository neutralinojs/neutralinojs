#include "hwinfo/platform.h"

#ifdef HWINFO_UNIX

#include <fstream>
#include <iterator>
#include <limits>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "hwinfo/monitoring/cpu.h"

namespace {

struct Jiffies {
  int64_t working = 0;
  int64_t all = 0;
};

// Reads a single cpu* line from /proc/stat (index 0 = aggregate "cpu" line,
// index 1 = "cpu0", index 2 = "cpu1", ...).
Jiffies get_jiffies(int index) {
  std::ifstream f("/proc/stat");
  if (!f.is_open()) return {};
  for (int i = 0; i < index; ++i) {
    if (!f.ignore(std::numeric_limits<std::streamsize>::max(), '\n')) return {};
  }
  std::string line;
  if (!std::getline(f, line)) return {};

  std::istringstream iss(line);
  std::vector<std::string> parts(std::istream_iterator<std::string>{iss},
                                  std::istream_iterator<std::string>{});
  if (parts.size() < 11) return {};

  int64_t v[10]{};
  for (int i = 0; i < 10; ++i) v[i] = std::stoll(parts[i + 1]);

  const int64_t all = v[0] + v[1] + v[2] + v[3] + v[4] + v[5] + v[6] + v[7] + v[8] + v[9];
  const int64_t working = v[0] + v[1] + v[2];
  return {working, all};
}

}  // namespace

namespace hwinfo::monitoring::cpu {

std::vector<double> thread_utilization(std::chrono::milliseconds sleep) {
  const unsigned n = std::thread::hardware_concurrency();
  // Snapshot before sleep
  std::vector<Jiffies> before(n);
  for (unsigned i = 0; i < n; ++i) before[i] = get_jiffies(static_cast<int>(i) + 1);

  std::this_thread::sleep_for(sleep);

  std::vector<double> result;
  result.reserve(n);
  for (unsigned i = 0; i < n; ++i) {
    const Jiffies after = get_jiffies(static_cast<int>(i) + 1);
    const double total = static_cast<double>(after.all - before[i].all);
    const double work = static_cast<double>(after.working - before[i].working);
    if (total <= 0.0) {
      result.push_back(0.0);
    } else {
      const double util = work / total;
      result.push_back((util < 0.0 || util > 1.0) ? 0.0 : util);
    }
  }
  return result;
}

double utilization(std::chrono::milliseconds sleep) {
  const auto tu = thread_utilization(sleep);
  if (tu.empty()) return 0.0;
  return std::accumulate(tu.begin(), tu.end(), 0.0) / static_cast<double>(tu.size());
}

std::vector<int64_t> thread_frequency_hz() {
  std::vector<int64_t> result;
  for (unsigned i = 0;; ++i) {
    std::ifstream f("/sys/devices/system/cpu/cpu" + std::to_string(i) + "/cpufreq/scaling_cur_freq");
    if (!f.is_open()) break;
    uint64_t khz = 0;
    f >> khz;
    result.push_back(static_cast<int64_t>(khz) * 1000LL);
  }
  return result;
}

Data fetch(std::chrono::milliseconds sleep) {
  auto tu = thread_utilization(sleep);
  auto tf = thread_frequency_hz();
  const double avg =
      tu.empty() ? 0.0 : std::accumulate(tu.begin(), tu.end(), 0.0) / static_cast<double>(tu.size());
  return Data{avg, std::move(tu), std::move(tf)};
}

}  // namespace hwinfo::monitoring::cpu

#endif  // HWINFO_UNIX
