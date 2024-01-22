/*
 * ipupdown - list IP addresses + network interface statuses
 *
 * Simple troubleshooting/scripting utility using standard
 * system libraries. Does not require other programs.
 * Just prints a basic IP/status summary.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h> // ETH_ALEN = 6, but in case ever changes

#include "common.h"

char *get_ip_address_string(struct ifreq *ifr, int sock);

int main()
{
    Interface interfaces[MAX_INTERFACES];
    Interface temp_interface;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket failed");
        return 1;
    }

    struct ifconf ifc;
    char buf[1024];
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) < 0) {
        perror("ioctl SIOCGIFCONF failed");
        close(sock);
        return 1;
    }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    char *status = "DOWN";
    char *ip_address_str;
    int if_name_longest_len = 0;
    int if_name_current_len = 0;
    int count = 0;
    for (; it != end && count < MAX_INTERFACES ; ++it) {
        struct ifreq ifr;
        strcpy(ifr.ifr_name, it->ifr_name);

        // Retrieve and store the MAC address
        if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
            perror("ioctl SIOCGIFHWADDR failed");
            continue;
        }

        // Retrieve and store the interface status
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
            perror("ioctl SIOCGIFFLAGS failed");
            continue;
        }
        status = (ifr.ifr_flags & IFF_UP) ? "UP" : "DOWN";
        strcpy(temp_interface.status, status);

        // Retrieve and store the IP address
        ip_address_str = get_ip_address_string(it, sock);
        store_ip_address(ip_address_str, &temp_interface);

        // Update the interface name and the longest interface name length
        store_interface_name(ifr.ifr_name, &temp_interface);

/*
        if_name_current_len = strlen(ifr.ifr_name);
        if (if_name_current_len > if_name_longest_len) {
            if_name_longest_len = if_name_current_len;
        }
*/
        // store the MAC address
        store_mac_address((unsigned char *)ifr.ifr_hwaddr.sa_data, ETH_ALEN, &temp_interface);

        // Store the current interface data in the array
        interfaces[count++] = temp_interface;
    }

    close(sock);
    print_interfaces(interfaces, count);
    return 0;
}

char *get_ip_address_string(struct ifreq *ifr, int sock)
{
    char *ip_address_string;

    if (ioctl(sock, SIOCGIFADDR, ifr) < 0) {
        ip_address_string = NULL; // "N/A" Indicate error
    } else {
        struct sockaddr_in* ipaddr = (struct sockaddr_in*)&ifr->ifr_addr;
        ip_address_string = inet_ntoa(ipaddr->sin_addr);
    }

    return ip_address_string;
}
