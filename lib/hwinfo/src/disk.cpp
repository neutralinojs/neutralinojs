// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#include <hwinfo/disk.h>

#include <ostream>

namespace hwinfo {

// _____________________________________________________________________________________________________________________
const std::string& Disk::vendor() const { return _vendor; }

// _____________________________________________________________________________________________________________________
const std::string& Disk::model() const { return _model; }

// _____________________________________________________________________________________________________________________
const std::string& Disk::serial_number() const { return _serial_number; }

// _____________________________________________________________________________________________________________________
uint64_t Disk::size() const { return _size_bytes; }

// _____________________________________________________________________________________________________________________
std::uint32_t Disk::id() const { return _id; }

// _____________________________________________________________________________________________________________________
const std::vector<std::string>& Disk::mount_points() const { return _mount_points; }

// _____________________________________________________________________________________________________________________
Disk::Interface Disk::disk_interface() const { return _interface; }

// =====================================================================================================================
// _____________________________________________________________________________________________________________________
std::ostream& operator<<(std::ostream& os, const Disk::Interface& disk_interface) {
  switch (disk_interface) {
    case Disk::Interface::NVME:
      os << "Disk::Interface::NVME";
      break;
    case hwinfo::Disk::Interface::USB:
      os << "Disk::Interface::USB";
      break;
    case Disk::Interface::USB1:
      os << "Disk::Interface::USB1";
      break;
    case Disk::Interface::USB2:
      os << "Disk::Interface::USB2";
      break;
    case Disk::Interface::USB3_5GBit:
      os << "Disk::Interface::USB3_5GBit";
      break;
    case Disk::Interface::USB3_10GBit:
      os << "Disk::Interface::USB3_10GBit";
      break;
    case Disk::Interface::USB3_20GBit:
      os << "Disk::Interface::USB3_20GBit";
      break;
    case Disk::Interface::USB4_20GBit:
      os << "Disk::Interface::USB4_20GBit";
      break;
    case Disk::Interface::USB4_40GBit:
      os << "Disk::Interface::USB4_40GBit";
      break;
    case Disk::Interface::USB4_80GBit:
      os << "Disk::Interface::USB3_80GBit";
      break;
    case Disk::Interface::SATA:
      os << "Disk::Interface::SATA";
      break;
    case Disk::Interface::SCSI:
      os << "Disk::Interface::SCSI";
      break;
    case Disk::Interface::UNKNOWN:
      os << "Disk::Interface::UNKNOWN";
      break;
  }
  return os;
}

}  // namespace hwinfo
