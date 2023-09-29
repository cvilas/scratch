#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <ifaddrs.h>
#include <netinet/ether.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <iostream>

int main()
{
	// Get all network interfaces in linux
	struct ifaddrs *addrs;
	getifaddrs(&addrs);
	for(struct ifaddrs *addr = addrs; addr != nullptr; addr = addr->ifa_next) {
		if(addr->ifa_addr && addr->ifa_addr->sa_family == AF_PACKET) {
			std::cout << "Inteface name: " << addr->ifa_name << std::endl;
			int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
			
			struct ifreq ifr{};
			strcpy(ifr.ifr_name, addr->ifa_name);
			
			// Read MAC address
			ioctl(fd, SIOCGIFHWADDR, &ifr);

                        char mac[18];
                        strcpy(mac, ether_ntoa((ether_addr *) ifr.ifr_hwaddr.sa_data));
                        std::cout << "MAC address:" << mac << std::endl;

                        // Read IP address
                        ioctl(fd, SIOCGIFADDR, &ifr);

                        char ip[INET_ADDRSTRLEN];
                        strcpy(ip, inet_ntoa(((sockaddr_in *) &ifr.ifr_addr)->sin_addr));
                        std::cout << "IP address: " << ip << std::endl << std::endl;
			close(fd);
		}
	}
	return 0;
}
