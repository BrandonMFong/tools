/**
 * author: Brando
 * date: 6/10/22
 */

#include <clib/clib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define HOSTNAME_BUF_LENGTH 200

/**
 * Loads addrs with all the ip addresses
 *
 * Caller owns addrs' memory
 */
int GetIPAddresses(char *** addrs, int * size) {
	int result = 0;
	char ** ipAddrs = 0;
	int arrSize = 0;
	struct ifaddrs * ifaddr = 0, * ifa = 0;
	char host[NI_MAXHOST];

	if (getifaddrs(&ifaddr) == -1) {
		result = 1;
		Error("Error with getifaddrs");
	}

	for (ifa = ifaddr; (ifa != NULL) && !result; ifa = ifa->ifa_next) {
		int family = ifa->ifa_addr->sa_family;

		if (family == AF_INET) {
			char * address = 0;

			if (getnameinfo(
				ifa->ifa_addr, 
				sizeof(struct sockaddr_in),
				host, 
				NI_MAXHOST, 
				NULL, 
				0, 
				NI_NUMERICHOST
			)) {
				result = 1;
			} else {
				address = CopyString(host, &result);
			}

			if (!result) {
				arrSize++;
				ipAddrs = (char **) realloc(ipAddrs, sizeof(char *) * arrSize);
				
				if (!ipAddrs) {
					result = -1;
				} else {
					ipAddrs[arrSize - 1] = address;
				}
			}
		}
	}

	if (addrs) *addrs = ipAddrs;
	if (size) *size = arrSize;

	return result;
}

int GetMacAddress(char * buffer, int bufSize) {
	int result = 0;
	struct ifreq ifr;
    	struct ifconf ifc;
	char buf[1024];

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock == -1) {
		result = 1;
		Error("Could not create socket");
	}

	if (!result) {
		ifc.ifc_len = sizeof(buf);
		ifc.ifc_buf = buf;
		if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) { 
			result = 1;
			Error("ioctl failed");
		}
	}

	if (!result) {
		struct ifreq * it = ifc.ifc_req;
		const struct ifreq * const end = it + (ifc.ifc_len / sizeof(struct ifreq));
		bool foundData = false;

		for (; !foundData && (it != end) && !result; ++it) {
			strcpy(ifr.ifr_name, it->ifr_name);
			if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
				if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
					if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
						foundData = true;
					}
				}
			}
			else {
				result = 1;
				Error("ioctl failed");
			}
		}
	}

	if (!result) {
		if (bufSize >= strlen(ifr.ifr_hwaddr.sa_data)) {
			memcpy(buf, ifr.ifr_hwaddr.sa_data, 6);
		}
	}

	return result;
}

int main(int argc, char * argv[]) {
	int result = 0;
	char hostname[HOSTNAME_BUF_LENGTH], ** addresses, macAddress[6];
	int addrArraySize = 0;

	if (gethostname(hostname, HOSTNAME_BUF_LENGTH)) {
		result = 1;
		Error("Error getting hostname for device");
	} else if (GetIPAddresses(&addresses, &addrArraySize)) {
		result = 1;
		Error("Error getting ip address for device");
	} else if (GetMacAddress(macAddress, sizeof(macAddress))) {
		result = 1;
		Error("Mac Address fetch error");
	} else {
		printf("Hostname: %s\n", hostname);
		printf("MAC Address: %s\n", macAddress);
		printf("IP Addresses: [ ");
		for(int i = 0; i < addrArraySize; i++) {
			printf("%s ", addresses[i]);
			free(addresses[i]);
		}
		printf("]\n");

		free(addresses);
	}

	return result;
}
