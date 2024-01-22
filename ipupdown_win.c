// compile with gcc -o .\ipupdown.exe .\ipupdown_win.c .\common.c -lws2_32 -liphlpapi

#include <ws2tcpip.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "common.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

void get_first_ipv4_address(PIP_ADAPTER_ADDRESSES pAdapter, char *ip_address_str);
PIP_ADAPTER_INFO FindMatchingAdapterInfo(PIP_ADAPTER_ADDRESSES pAddr, PIP_ADAPTER_INFO pInfo);

int main()
{
    WSADATA wsaData;
    DWORD dwRetVal = 0;
    ULONG outBufLen = 15000;
    PIP_ADAPTER_ADDRESSES pAddresses = NULL, pCurrAddresses = NULL;
    PIP_ADAPTER_INFO pAdapterInfo = NULL, pCurrAdapterInfo = NULL;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr,"WSAStartup failed.\n");
        return 1;
    }

    // Allocate memory for IP_ADAPTER_ADDRESSES structures
    pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
    if (pAddresses == NULL) {
        fprintf(stderr,"Unable to allocate memory for adapter addresses.\n");
        WSACleanup();
        return 1;
    }

    // Retrieve the adapter addresses
    if ((dwRetVal = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pAddresses, &outBufLen)) == ERROR_BUFFER_OVERFLOW) {
        free(pAddresses);
        pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
        if (pAddresses == NULL) {
            fprintf(stderr,"Unable to allocate memory for adapter addresses.\n");
            WSACleanup();
            return 1;
        }
        dwRetVal = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pAddresses, &outBufLen);
    }

    if (dwRetVal != NO_ERROR) {
        fprintf(stderr,"GetAdaptersAddresses failed with error: %u\n", dwRetVal);
        if (pAddresses) free(pAddresses);
        WSACleanup();
        return 1;
    }

    // Allocate memory for IP_ADAPTER_INFO structures
    outBufLen = sizeof(IP_ADAPTER_INFO);
    pAdapterInfo = (IP_ADAPTER_INFO*)malloc(outBufLen);
    if (pAdapterInfo == NULL) {
        fprintf(stderr,"Unable to allocate memory for adapter info.\n");
        WSACleanup();
        return 1;
    }

    // Retrieve the adapter info
    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &outBufLen)) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(outBufLen);
        if (pAdapterInfo == NULL) {
            fprintf(stderr,"Unable to allocate memory for adapter info.\n");
            WSACleanup();
            return 1;
        }
        dwRetVal = GetAdaptersInfo(pAdapterInfo, &outBufLen);
    }

    if (dwRetVal != NO_ERROR) {
        fprintf(stderr,"GetAdaptersInfo failed with error: %u\n", dwRetVal);
        if (pAdapterInfo) free(pAdapterInfo);
        WSACleanup();
        return 1;
    }

    // Initialize array of structs common to both Linux and Windows
    Interface interfaces[MAX_INTERFACES];
    char ip_address[MAX_IP_LEN];
    int count = 0;

    // Iterate over the adapter list and store information
    for (PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
            pCurrAddresses != NULL && count < MAX_INTERFACES;
            pCurrAddresses = pCurrAddresses->Next) {

        PIP_ADAPTER_INFO pMatchingAdapterInfo = FindMatchingAdapterInfo(pCurrAddresses, pAdapterInfo);
        if (pMatchingAdapterInfo == NULL) {
            continue;  // Skip if no matching adapter info found
        }

        Interface temp_interface;

        // Store status
        store_status(pCurrAddresses->OperStatus == IfOperStatusUp ? "Up" : "Down", &temp_interface);

        // Store IP address
        get_first_ipv4_address(pCurrAddresses, ip_address);
        store_ip_address(ip_address, &temp_interface);

        // Store MAC address
        store_mac_address(pMatchingAdapterInfo->Address, pMatchingAdapterInfo->AddressLength, &temp_interface);

        // Store interface name
        store_interface_name(pMatchingAdapterInfo->Description, &temp_interface);

        // Store temp struct in array
        interfaces[count++] = temp_interface;
    }

    print_interfaces(interfaces, count);  // Print the collected interface information

    // Cleanup
    if (pAddresses) free(pAddresses);
    if (pAdapterInfo) free(pAdapterInfo);
    WSACleanup();

    return 0;
}

// Helper function to find matching adapter in the GetAdaptersInfo list
PIP_ADAPTER_INFO FindMatchingAdapterInfo(PIP_ADAPTER_ADDRESSES pAddr, PIP_ADAPTER_INFO pInfo)
{
    while (pInfo != NULL) {
        if (strcmp(pAddr->AdapterName, pInfo->AdapterName) == 0) {
            return pInfo;
        }
        pInfo = pInfo->Next;
    }
    return NULL;
}

// Windows-specific method to retrieve ip address
void get_first_ipv4_address(PIP_ADAPTER_ADDRESSES pAdapter, char *ip_address_str)
{
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pAdapter->FirstUnicastAddress;
    BOOL ipAssigned = FALSE;

    while (pUnicast != NULL && !ipAssigned) {
        if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
            struct sockaddr_in* sockaddr_ipv4 = (struct sockaddr_in*)pUnicast->Address.lpSockaddr;
            inet_ntop(AF_INET, &sockaddr_ipv4->sin_addr, ip_address_str, MAX_IP_LEN);
            ipAssigned = TRUE;
        }
        pUnicast = pUnicast->Next;
    }

    if (!ipAssigned) {
        strncpy(ip_address_str, "N/A", MAX_IP_LEN);
        ip_address_str[MAX_IP_LEN - 1] = '\0'; // Ensure null termination
    }
}
