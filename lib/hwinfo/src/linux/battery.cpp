// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#include "hwinfo/platform.h"

#ifdef HWINFO_UNIX

#include <filesystem>
#include <fstream>

#include "hwinfo/battery.h"

namespace hwinfo {

static std::filesystem::path base_path("/sys/class/power_supply/");

namespace {

// _____________________________________________________________________________________________________________________
std::string get_vendor(std::uint32_t id) {
  std::ifstream vendor_file(base_path / ("BAT" + std::to_string(id)) / "manufacturer");
  std::string vendor;
  if (vendor_file.is_open()) {
    getline(vendor_file, vendor);
    return vendor;
  }
  return "<unknown>";
}

// _____________________________________________________________________________________________________________________
std::string get_model(std::uint32_t id) {
  std::ifstream vendor_file(base_path / ("BAT" + std::to_string(id)) / "model_name");
  std::string value;
  if (vendor_file.is_open()) {
    getline(vendor_file, value);
    return value;
  }
  return "<unknown>";
}

// _____________________________________________________________________________________________________________________
std::string get_serial_number(std::uint32_t id) {
  std::ifstream vendor_file(base_path / ("BAT" + std::to_string(id)) / "serial_number");
  std::string value;
  if (vendor_file.is_open()) {
    getline(vendor_file, value);
    return value;
  }
  return "<unknown>";
}

// _____________________________________________________________________________________________________________________
std::string get_technology(std::uint32_t id) {
  std::ifstream vendor_file(base_path / ("BAT" + std::to_string(id)) / "technology");
  std::string value;
  if (vendor_file.is_open()) {
    getline(vendor_file, value);
    return value;
  }
  return "<unknown>";
}

}  // namespace

// =====================================================================================================================
// _____________________________________________________________________________________________________________________
uint32_t get_energy_full(std::uint32_t id) {
  auto path = std::filesystem::path(base_path / ("BAT" + std::to_string(id)) / "energy_full");
  if (!std::filesystem::exists(path)) {
    path = std::filesystem::path(base_path / ("BAT" + std::to_string(id)) / "charge_full");
  }
  std::ifstream vendor_file(path);
  std::string value;
  if (vendor_file.is_open()) {
    getline(vendor_file, value);
    try {
      return std::stoi(value);
    } catch (const std::invalid_argument& e) {
      return 0;
    }
  }
  return 0;
}

// _____________________________________________________________________________________________________________________
uint32_t Battery::energyNow() const {
  auto path = std::filesystem::path(base_path / ("BAT" + std::to_string(_id)) / "energy_now");
  if (!std::filesystem::exists(path)) {
    path = std::filesystem::path(base_path / ("BAT" + std::to_string(_id)) / "charge_now");
  }
  std::ifstream vendor_file(path);
  std::string value;
  if (vendor_file.is_open()) {
    getline(vendor_file, value);
    try {
      return std::stoi(value);
    } catch (const std::invalid_argument& e) {
      return 0;
    }
  }
  return 0;
}

// _____________________________________________________________________________________________________________________
Battery::State Battery::state() const {
  std::ifstream vendor_file(base_path / ("BAT" + std::to_string(_id)) / "status");
  std::string value;
  if (vendor_file.is_open()) {
    getline(vendor_file, value);
    return value == "Charging" ? State::CHARGING : State::DISCHARGING;
  }
  return State::UNKNOWN;
}

// =====================================================================================================================
// _____________________________________________________________________________________________________________________
std::vector<Battery> getAllBatteries() {
  std::vector<Battery> batteries;
  std::uint32_t id = 0;
  while (std::filesystem::exists(base_path / ("BAT" + std::to_string(id)))) {
    batteries.emplace_back(id++);
    auto& battery = batteries.back();
    battery._vendor = get_vendor(id);
    battery._model = get_model(id);
    battery._energyFull = get_energy_full(id);
    battery._serial_number = get_serial_number(id);
    battery._technology = get_technology(id);
  }
  return batteries;
}

}  // namespace hwinfo

#endif
