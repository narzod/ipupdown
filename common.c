#include "common.h"

int compare_interfaces_by_ip_address(const void *a, const void *b)
{
    const Interface *interface1 = (const Interface *)a;
    const Interface *interface2 = (const Interface *)b;

    return strcmp(interface1->ip_address, interface2->ip_address);
}

int compare_interfaces(const void *a, const void *b)
{
    const Interface *interface1 = (const Interface *)a;
    const Interface *interface2 = (const Interface *)b;

    // Prioritize sorting by status first
    int status_comparison = strcmp(interface1->status, interface2->status);
    if (status_comparison != 0) {
        return status_comparison; // Different statuses, sort based on that
    }

    // If statuses are the same, sort by IP address
    return strcmp(interface1->ip_address, interface2->ip_address);
}

void print_interfaces(Interface interfaces[], const int num_interfaces)
{
    qsort(interfaces, num_interfaces, sizeof(Interface), compare_interfaces);
    for (int i = 0; i < num_interfaces; i++) {
        printf("%-4s  %-15s  %s  %s\n",
               interfaces[i].status,
               interfaces[i].ip_address,
               interfaces[i].mac_address,
               interfaces[i].name
              );
    }
}

void store_status(const char *status, Interface *adapter)
{
    strcpy(adapter->status, status);
}

void store_ip_address(const char *ip_address, Interface *adapter)
{
    strncpy(adapter->ip_address, ip_address, MAX_IP_LEN - 1);
    adapter->ip_address[MAX_IP_LEN - 1] = '\0'; // Ensure null termination
}

void store_mac_address(const uint8_t *addr, int len, Interface *adapter)
{
    char *mac_addr = adapter->mac_address;
    int offset = 0;

    for (int i = 0; i < len; i++) {
        if (i > 0) {
            mac_addr[offset++] = ':';
        }
        sprintf(mac_addr + offset, "%02x", addr[i]);
        offset += 2; // Each byte contributes two hex characters
    }

    mac_addr[offset] = '\0'; // Null-terminate the string
}

void store_interface_name(const char *name, Interface *adapter)
{
    strncpy(adapter->name, name, MAX_NAME_LEN - 1);
    adapter->name[MAX_NAME_LEN - 1] = '\0'; // Ensure null termination
}

