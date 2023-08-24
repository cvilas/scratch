#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main() {
    const char *hostname = "clemson.edu";  // A service to get your external IP address
    const char *port = "80";  // HTTP port

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP socket

    s = getaddrinfo(hostname, port, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    // Iterate through results and try to connect
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1) {
            continue;
        }

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            break;  // Successfully connected
        }

        close(sfd);
    }

    if (rp == NULL) {
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);

    // Use getnameinfo() to get the IP address as a string
    char ipstr[INET6_ADDRSTRLEN];
    void *addr;

    if (rp->ai_family == AF_INET) {  // IPv4
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)rp->ai_addr;
        addr = &(ipv4->sin_addr);
    } else {  // IPv6
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)rp->ai_addr;
        addr = &(ipv6->sin6_addr);
    }

    inet_ntop(rp->ai_family, addr, ipstr, sizeof(ipstr));
    printf("External IP Address: %s\n", ipstr);

    close(sfd);

    return 0;
}

