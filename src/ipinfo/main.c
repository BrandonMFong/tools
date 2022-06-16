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
	struct ifaddrs *ifaddr, *ifa;
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

int main(int argc, char * argv[]) {
	int result = 0;
	char hostname[HOSTNAME_BUF_LENGTH], ** addresses;
	int addrArraySize = 0;

	if (gethostname(hostname, HOSTNAME_BUF_LENGTH)) {
		result = 1;
		Error("Error getting hostname for device");
	} else if (GetIPAddresses(&addresses, &addrArraySize)) {
		result = 1;
		Error("Error getting ip address for device");
	} else {
		printf("Hostname: %s\n", hostname);

		printf("Addresses: [ ");
		for(int i = 0; i < addrArraySize; i++) {
			printf("%s ", addresses[i]);
			free(addresses[i]);
		}
		printf("]\n");

		free(addresses);
	}

	return result;
}
