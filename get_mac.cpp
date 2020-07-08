#include <iostream>
#include <array>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <net/if.h>
#include <cstring>
#include <unistd.h>

auto getMacAddress(const std::string &iface) -> std::array<uint8_t, 6> {
	int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy((char *)ifr.ifr_name , iface.c_str() , IFNAMSIZ-1);
    ioctl(fd, SIOCGIFHWADDR, &ifr);
    ::close(fd);
    std::array<uint8_t, 6> mac;
    std::memcpy(mac.data(), (char*)ifr.ifr_hwaddr.sa_data,6);
    return mac;
}

int main(int argc, char* argv[]) {
    const auto mac = getMacAddress(argv[1]);
    for(const auto val : mac) {
        std::cout << std::hex << (int)val << " ";
    }
    return 0;
}
