#include <hwinfo/platform.h>

#ifdef HWINFO_UNIX
#include <arpa/inet.h>
#include <hwinfo/network.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>

#include <cstring>
#include <fstream>
#include <string>
#include <vector>

namespace hwinfo {

std::string getInterfaceIndex(const std::string& path) {
  int index = if_nametoindex(path.c_str());
  return (index > 0) ? std::to_string(index) : "<unknown>";
}

std::string getDescription(const std::string& path) { return path; }

std::string getMac(const std::string& path) {
  std::ifstream file("/sys/class/net/" + path + "/address");
  std::string mac;
  if (file.is_open()) {
    std::getline(file, mac);
    file.close();
  }
  return mac.empty() ? "<unknown>" : mac;
}

std::string getIp4(const std::string& interface) {
  std::string ip4 = "<unknown>";
  struct ifaddrs* ifaddr;
  if (getifaddrs(&ifaddr) == -1) {
    return ip4;
  }

  for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == nullptr) continue;
    if (ifa->ifa_addr->sa_family == AF_INET && interface == ifa->ifa_name) {
      char ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr, ip, sizeof(ip));
      ip4 = ip;
      break;
    }
  }
  freeifaddrs(ifaddr);
  return ip4;
}

std::string getIp6(const std::string& interface) {
  std::string ip6 = "<unknown>";
  struct ifaddrs* ifaddr;
  if (getifaddrs(&ifaddr) == -1) {
    return ip6;
  }

  for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == nullptr) {
      continue;
    }
    if (ifa->ifa_addr->sa_family == AF_INET6 && interface == ifa->ifa_name) {
      char ip[INET6_ADDRSTRLEN];
      inet_ntop(AF_INET6, &((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr, ip, sizeof(ip));
      if (std::strncmp(ip, "fe80", 4) == 0) {
        ip6 = ip;
        break;
      }
    }
  }
  freeifaddrs(ifaddr);
  return ip6;
}

std::vector<Network> getAllNetworks() {
  std::vector<Network> networks;
  struct ifaddrs* ifaddr;
  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    return networks;
  }

  for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == nullptr) continue;
    if (ifa->ifa_addr->sa_family != AF_PACKET) continue;

    std::string interface = ifa->ifa_name;
    Network network;
    network._index = getInterfaceIndex(interface);
    network._description = getDescription(interface);
    network._mac = getMac(interface);
    network._ip4 = getIp4(interface);
    network._ip6 = getIp6(interface);
    networks.push_back(std::move(network));
  }
  freeifaddrs(ifaddr);
  return networks;
}

}  // namespace hwinfo

#endif
