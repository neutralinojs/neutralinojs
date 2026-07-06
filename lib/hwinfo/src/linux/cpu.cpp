// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include <hwinfo/platform.h>

#ifdef HWINFO_UNIX

#include <chrono>
#include <cmath>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include "hwinfo/cpu.h"
#include "hwinfo/unix/cpu.h"
#include "hwinfo/utils/stringutils.h"
#include "hwinfo/utils/unit.h"

struct Jiffies {
  Jiffies() {
    working = -1;
    all = -1;
  }

  Jiffies(int64_t _all, int64_t _working) {
    all = _all;
    working = _working;
  }

  int64_t working;
  int64_t all;
};

namespace {

bool jiffies_initialized = false;

Jiffies get_jiffies(int index) {
  std::ifstream filestat("/proc/stat");
  if (!filestat.is_open()) {
    return {};
  }

  for (int i = 0; i < index; ++i) {
    if (!filestat.ignore(std::numeric_limits<std::streamsize>::max(), '\n')) {
      break;
    }
  }
  std::string line;
  std::getline(filestat, line);

  std::istringstream iss(line);
  std::vector<std::string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());

  const int64_t jiffies_0 = std::stol(results[1]);
  const int64_t jiffies_1 = std::stol(results[2]);
  const int64_t jiffies_2 = std::stol(results[3]);
  const int64_t jiffies_3 = std::stol(results[4]);
  const int64_t jiffies_4 = std::stol(results[5]);
  const int64_t jiffies_5 = std::stol(results[6]);
  const int64_t jiffies_6 = std::stol(results[7]);
  const int64_t jiffies_7 = std::stol(results[8]);
  const int64_t jiffies_8 = std::stol(results[9]);
  const int64_t jiffies_9 = std::stol(results[10]);

  int64_t all = jiffies_0 + jiffies_1 + jiffies_2 + jiffies_3 + jiffies_4 + jiffies_5 + jiffies_6 + jiffies_7 +
                jiffies_8 + jiffies_9;
  int64_t working = jiffies_0 + jiffies_1 + jiffies_2;

  return {all, working};
}

}  // namespace

namespace hwinfo {

inline uint64_t readSysfsUint(const std::string& path) {
  std::ifstream f(path);
  uint64_t val = 0;
  if (f >> val) {
    return val;
  }
  return 0;
}

inline std::uint64_t read_cache_size(const std::string& path) {
  std::ifstream f(path);
  if (!f.is_open()) {
    return 0;
  }
  std::uint64_t value;
  std::string unit;
  if (f >> value >> unit) {
    if (unit == "K") {
      return value * static_cast<std::uint64_t>(unit::IECPrefix::KIBI);
    } else if (unit == "M") {
      return value * static_cast<std::uint64_t>(unit::IECPrefix::MEBI);
    } else if (unit == "G") {
      return value * static_cast<std::uint64_t>(unit::IECPrefix::GIBI);
    }
    return value;
  }
  return 0;
}

inline bool has_smt(const std::string& core_path) {
  std::string path = core_path + "/topology/thread_siblings_list";
  std::ifstream tsf(path);
  std::string siblings;
  return (tsf >> siblings && siblings.find_first_of('-') != std::string::npos);
}

namespace unix_os::cpu {

std::map<std::uint32_t, std::map<std::uint32_t, std::map<std::string, std::string>>> parse_cpuinfo_file(
    const std::string& data) {
  std::vector<std::map<std::string, std::string>> blocks;
  {
    std::string fallback_vendor;
    std::string fallback_model;
    std::vector<std::string> sections = utils::split(data, "\n\n");
    for (const auto& section : sections) {
      std::map<std::string, std::string> current_block;
      for (const auto& line : utils::split(section, "\n")) {
        std::vector<std::string> pair = utils::split(line, ":");
        if (pair.size() != 2) {
          continue;
        }
        std::string key = std::move(pair[0]);
        std::string value = std::move(pair[1]);
        utils::strip(key);
        utils::strip(value);
        current_block[key] = value;
      }
      {
        // On some ARM systems, Processor and Hardware are defined globally for all listed cores
        if (current_block.count("Processor")) {
          fallback_model = current_block["Processor"];
        } else if (current_block.count("Hardware")) {
          fallback_vendor = current_block["Hardware"];
        } else if (current_block.count("Model")) {
          fallback_model = current_block["Model"];
        }
      }
      blocks.emplace_back(std::move(current_block));
    }

    std::map<std::uint32_t, std::map<std::uint32_t, std::map<std::string, std::string>>> cpus;
    for (auto& block : blocks) {
      if (block.count("processor") == 0) {
        // skip as this block is not an entry for a core
        continue;
      }
      std::uint32_t processor_id = std::stoi(block["processor"]);
      std::uint32_t physical_id = 0;
      if (block.count("physical id")) {
        physical_id = std::stoi(block["physical id"]);
      }
      block["fallback_model"] = fallback_model;
      block["fallback_vendor"] = fallback_vendor;
      auto& socket = cpus[physical_id];
      socket[processor_id].insert(std::move_iterator(block.begin()), std::move_iterator(block.end()));
    }
    return cpus;
  }
}

std::map<std::uint32_t, unix_os::cpu::CPU> parse_cpus(
    const std::map<std::uint32_t, std::map<std::uint32_t, std::map<std::string, std::string>>>& sockets) {
  if (sockets.empty()) {
    return {};
  }
  std::map<std::uint32_t, unix_os::cpu::CPU> cpus;
  for (const auto& [socket_id, processors] : sockets) {
    unix_os::cpu::CPU cpu;
    cpu.socket_id = socket_id;
    for (const auto& [processor_id, processor] : processors) {
      std::uint32_t core_id;
      if (processor.count("core id")) {
        core_id = std::stoi(processor.at("core id"));
      } else {
        core_id = processor_id;
      }
      if (cpu.cores.count(core_id)) {
        continue;
      }
      {  // CPU information
        if (processor.count("vendor_id")) {
          cpu.vendor = processor.at("vendor_id");
        } else {
          if (cpu.vendor.empty()) {
            cpu.vendor = processor.at("fallback_vendor");
          }
        }
        if (processor.count("model name")) {
          cpu.model = processor.at("model name");
        } else {
          if (cpu.model.empty()) {
            cpu.model = processor.at("fallback_model");
          }
        }
      }
      {  // core information
        unix_os::cpu::CPU::Core& core = cpu.cores[core_id];
        const std::string core_path = "/sys/devices/system/cpu/cpu" + std::to_string(processor_id);
        core.id = processor_id;
        core.smt = has_smt(core_path);
        core.cache_bytes[0] = read_cache_size(core_path + "/cache/index0/size");
        core.cache_bytes[1] = read_cache_size(core_path + "/cache/index1/size");
        core.cache_bytes[2] = read_cache_size(core_path + "/cache/index2/size");
        core.cache_bytes[3] = read_cache_size(core_path + "/cache/index3/size");
        if (processor.count("flags")) {
          core.flags = utils::split(processor.at("flags"), " ");
        } else if (processor.count("Features")) {
          core.flags = utils::split(processor.at("Features"), " ");
        } else if (processor.count("isa")) {
          core.flags = utils::split(processor.at("isa"), " ");
        }
        core.regular_frequency_hz = readSysfsUint(core_path + "/cpufreq/base_frequency");
        if (core.regular_frequency_hz == 0 && processor.count("cpu MHz")) {
          core.regular_frequency_hz = std::stod(processor.at("cpu MHz")) * unit::SiPrefix::MEGA;
        }
        core.max_frequency_hz = readSysfsUint(core_path + "/cpufreq/cpuinfo_max_freq");
      }
    }
    cpus[socket_id] = std::move(cpu);
  }
  return cpus;
}

}  // namespace unix_os::cpu

namespace monitor::cpu {

// _____________________________________________________________________________________________________________________
std::vector<std::uint64_t> currentClockSpeed_Hz() {
  std::vector<std::uint64_t> res;
  for (int core_id = 0; /* breaks, if i is no valid cpu id */; ++core_id) {
    std::uint64_t frequency_hz =
        readSysfsUint("/sys/devices/system/cpu/cpu" + std::to_string(core_id) + "/cpufreq/scaling_cur_freq");
    if (frequency_hz == std::numeric_limits<std::uint64_t>::max()) {
      break;
    }
    res.push_back(frequency_hz);
  }

  return res;
}

// _____________________________________________________________________________________________________________________
void init_jiffies() {
  if (!jiffies_initialized) {
    // Sleep 1 sec just for the start cause the usage needs to have a delta value which is depending on the unix file
    // read it's just for the init, you don't need to wait if the delta is already created ...
    std::this_thread::sleep_for(1s);
    jiffies_initialized = true;
  }
}

// _____________________________________________________________________________________________________________________
double utilization([[maybe_unused]] std::chrono::milliseconds sleep) {
  init_jiffies();
  // TODO: Leon Freist a socket max num and a socket id inside the CPU could make it work with all sockets
  //       I will not support it because I only have a 1 socket target device
  static Jiffies last = Jiffies();

  Jiffies current = get_jiffies(0);

  auto total_over_period = static_cast<double>(current.all - last.all);
  auto work_over_period = static_cast<double>(current.working - last.working);

  last = current;

  const double utilization = work_over_period / total_over_period;
  if (utilization < 0 || utilization > 1 || std::isnan(utilization)) {
    return -1.0;
  }
  return utilization;
}

// _____________________________________________________________________________________________________________________
double coreUtilization(int thread_index) {
  init_jiffies();
  // TODO: Leon Freist a socket max num and a socket id inside the CPU could make it work with all sockets
  //       I will not support it because I only have a 1 socket target device
  static std::vector<Jiffies> last(0);
  if (last.empty()) {
    last.resize(std::thread::hardware_concurrency());
  }

  Jiffies current = get_jiffies(thread_index + 1);  // thread_index works only with 1 socket right now

  auto total_over_period = static_cast<double>(current.all - last[thread_index].all);
  auto work_over_period = static_cast<double>(current.working - last[thread_index].working);

  last[thread_index] = current;

  const double percentage = work_over_period / total_over_period;
  if (percentage < 0 || percentage > 100 || std::isnan(percentage)) {
    return -1.0;
  }
  return percentage;
}

// _____________________________________________________________________________________________________________________
std::vector<double> core_utilization() {
  std::vector<double> thread_utility(std::thread::hardware_concurrency());
  for (std::size_t thread_idx = 0; thread_idx < thread_utility.size(); ++thread_idx) {
    thread_utility[thread_idx] = coreUtilization(thread_idx);
  }
  return thread_utility;
}

}  // namespace monitor::cpu

// CPU Temp -> Works | But requires Im_sensors
// double CPU::currentTemperature_Celsius() const {
//     if (!std::ifstream("/etc/sensors3.conf"))
//     {
//       std::cout << "The lm-sensors, the tool for monitoring your system's temperature, needs to be configured. Please
//       set it up." << '\n';
//       // Configure lm-sensors if not already configured
//       std::string detect_command = "sudo sensors-detect";
//       std::system(detect_command.c_str());
//     }

//     // TODO: Leon Freist a socket max num and a socket id inside the CPU could make it work with all sockets
//     //       I will not support it because I only have a 1 socket target device
//     const int Socked_id = 0;

//     // Command to get temperature data using 'sensors' command
//     std::string command = "sensors | grep 'Package id " + std::to_string(Socked_id) + "' | awk '{print $4}'";

//     // Open a pipe to execute the command and capture its output
//     FILE* pipe = popen(command.c_str(), "r");
//     if (!pipe) {
//         std::cerr << "Error executing command." << '\n';
//         return -1.0; // Return a negative value to indicate an error
//     }

//     char buffer[128];
//     std::string result = "";

//     // Read the output of the command into 'result'
//     while (!feof(pipe)) {
//         if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
//             result += buffer;
//         }
//     }

//     // Close the pipe
//     pclose(pipe);

//     // Convert the result (string) to a double
//     double temperature = -1.0; // Default value in case of conversion failure
//     std::istringstream(result) >> temperature;

//     return temperature;
// }

// =====================================================================================================================
// _____________________________________________________________________________________________________________________
std::vector<CPU> getAllCPUs() {
  std::ifstream file("/proc/cpuinfo");
  if (!file.is_open()) return {};

  std::string data(std::istreambuf_iterator<char>{file}, {});

  auto blocks = unix_os::cpu::parse_cpuinfo_file(data);
  auto parsed_cpus = unix_os::cpu::parse_cpus(blocks);

  std::vector<CPU> result;
  result.reserve(parsed_cpus.size());
  for (const auto& [socket_id, parsed_cpu] : parsed_cpus) {
    CPU cpu;
    cpu._vendor = parsed_cpu.vendor;
    cpu._modelName = parsed_cpu.model;
    cpu._id = socket_id;
    cpu._cores.reserve(parsed_cpus.size());
    for (const auto& [core_id, core] : parsed_cpu.cores) {
      cpu._cores.emplace_back(CPU::Core{
          core.id, CPU::Cache{core.cache_bytes[0], core.cache_bytes[1], core.cache_bytes[2], core.cache_bytes[3]},
          core.regular_frequency_hz, core.max_frequency_hz, core.smt});

      cpu._numLogicalCores += core.smt ? 2 : 1;
      cpu._numPhysicalCores += 1;
    }
    if (!parsed_cpu.cores.empty()) {
      cpu._flags = parsed_cpu.cores.begin()->second.flags;
    }
    result.emplace_back(std::move(cpu));
  }

  return result;
}

}  // namespace hwinfo

#endif  // HWINFO_UNIX
