#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // Include this for uint8_t

#define MAX_INTERFACES      50
#define MAX_NAME_LEN        50
#define PADDING             25

// Length limits for MAC addresses and IP4 addresses
#define MAX_MAC_LEN         18      // 17 for digits & colons + NULL
#define MAX_IP_LEN          16      // 15 for digits & 3 dots + NULL

typedef struct {
    char name[MAX_NAME_LEN];
    char mac_address[MAX_MAC_LEN];  // Format: XX:XX:XX:XX:XX:XX
    char ip_address[MAX_IP_LEN];    // Format: NNN.NNN.NNN.NNN
    char status[5];                 // "UP", "DOWN"
} Interface;

int compare_interfaces_by_ip_address(const void *a, const void *b);
int compare_interfaces(const void *a, const void *b);
void print_interfaces(Interface interfaces[], const int num_interfaces);

void store_status(const char *status, Interface *adapter);
void store_ip_address(const char *ip_address, Interface *adapter);
void store_mac_address(const uint8_t *addr, int len, Interface *adapter);
void store_interface_name(const char *name, Interface *adapter);

#endif // COMMON_H
