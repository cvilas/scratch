#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#ifdef __APPLE__
#include <net/if_dl.h>
#else
#include <netinet/ether.h>
#include <linux/if_packet.h>
#endif

std::string getMacAddress(const struct ifaddrs* addr) {
#if defined(__APPLE__)
    // On macOS, MAC address is in AF_LINK family
    if (addr->ifa_addr->sa_family != AF_LINK) {
			return "";
		}
		struct sockaddr_dl* sdl = (struct sockaddr_dl*)addr->ifa_addr;
		unsigned char mac[6];
		memcpy(mac, LLADDR(sdl), sdl->sdl_alen);
		char mac_str[18];
		snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
						mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		return std::string(mac_str);
#elif defined(__linux__)
    // On Linux, use ioctl to get MAC address
    const int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (fd < 0) {
			return "";
		}

    struct ifreq ifr{};
    strncpy(ifr.ifr_name, addr->ifa_name, IFNAMSIZ-1);
    
		const auto ret = ioctl(fd, SIOCGIFHWADDR, &ifr);
		close(fd);
    if ( ret != 0) {
			return "";
		}
		char mac[18];
		strcpy(mac, ether_ntoa((ether_addr *) ifr.ifr_hwaddr.sa_data));
		return std::string(mac);
#endif
return "";
}

std::string getIpAddress(const char* interface_name) {
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (fd < 0) {
			return "";
		}

    struct ifreq ifr;
    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ-1);
    
		const auto ret = ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    if ( ret != 0) {
			return "";
		}
    return inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr);
}

int main() {
    struct ifaddrs *addrs;
    if (getifaddrs(&addrs) == -1) {
        std::cerr << "Failed to get network interfaces" << std::endl;
        return 1;
    }
#if defined(__APPLE__)
  #ifndef AF_PACKET
    #define AF_PACKET AF_LINK
  #endif
#endif
    for (struct ifaddrs *addr = addrs; addr != nullptr; addr = addr->ifa_next) {
        if ((addr->ifa_addr == nullptr) or addr->ifa_addr->sa_family != AF_PACKET) continue;

        // Get interface name
        std::cout << "Interface name: " << addr->ifa_name << std::endl;
        
        // Get MAC address
        std::string mac = getMacAddress(addr);
        if (!mac.empty()) {
            std::cout << "MAC address: " << mac << std::endl;
        }

        // Get IP address
        std::string ip = getIpAddress(addr->ifa_name);
        if (!ip.empty()) {
            std::cout << "IP address: " << ip << std::endl;
        }

        std::cout << std::endl;
    }

    freeifaddrs(addrs);
    return 0;
}