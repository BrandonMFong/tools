/**
 * author: Brando
 * date: 6/10/22
 */

#include <clib/clib.h>
#include <stdio.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <net/if.h>
#include <stdbool.h>

#ifdef LINUX 
#include <linux/if_packet.h>
#elif OSX
#include <net/if_dl.h>
#endif

#ifdef LINUX
#define AF_HW AF_PACKET
#elif OSX
#define AF_HW AF_LINK
#else 
#error Unknown OS
#endif

struct NetInterface {
	char _name[50];

	char _ipaddr[(3 * 4) + 4];
	char _macaddr[(6 * 2) + 6];

	struct NetInterface * _next;
};

/**
 * Creates a copy of the mac address
 *
 * Caller must free
 */
char * CopyMacAddress(struct sockaddr * sa, int * err) {
	char * result = 0;
	int error = 0;
	char buf[20]; // buffer for the mac address string
	unsigned char * s = 0;

	memset(&buf[0], 0, 20);

#ifdef LINUX
	//struct sockaddr_ll * saddr = (struct sockaddr_ll *) sa;
	s = ((struct sockaddr_ll *) sa)->sll_addr;

	/*
	int l = 0;
	for (int i = 0; i < 6; i++) {
		l += sprintf(buf+l, "%02X%s", saddr->sll_addr[i], i < 5 ? ":" : "");
	}
	*/
#elif OSX
	s = (unsigned char *) LLADDR((struct sockaddr_dl *) sa);

	/*
	if (t) {
		int l = 0;
		for (int i = 0; i < 6; i++) {
			l += sprintf(buf+l, "%02X%s", t[i], i < 5 ? ":" : "");
		}
	} else {
		error = 1;
		Error("Error with LLADDR");
	}
	*/
#endif
	if (s == 0) {
		error = 1;
		Error("Could not get raw data");
	}

	if (!error) {
		int l = 0;
		for (int i = 0; i < 6; i++) {
			l += sprintf(buf+l, "%02X%s", s[i], i < 5 ? ":" : "");
		}

		result = CopyString(buf, &error);
	}

	if (err != 0) {
		*err = error;
	}

	return result;
}

int main() {
	int result = 0;
	struct ifaddrs * addrs, * tmp;
	struct sockaddr_in * sa;
	struct NetInterface * nifRoot = 0;

	if (getifaddrs(&addrs) == -1) {
		Error("Could not get interfaces");
		result = 1;
	} else {
		tmp = addrs;
	}

	if (!result) {
		// First get the ip addresses
		do {
			bool cont = false;
			struct NetInterface * nif = 0;

			if (		tmp->ifa_addr  // Valid address
				&& 	(tmp->ifa_flags & IFF_UP) // The interface is active
				&& 	!(tmp->ifa_flags & IFF_LOOPBACK)) { // Interface is not a loopback
				if (tmp->ifa_addr->sa_family == AF_INET) {
					cont = true; // Found an interface with an IP address
				}
			}

			if (cont) {
				if (!result) {
					nif = (struct NetInterface *) malloc(sizeof(struct NetInterface));
					result = nif != 0 ? 0 : 1;

					if (nif == 0) {
						result = 1;
					} else {
						memset(nif, 0, sizeof(struct NetInterface));
						
						// Copy the interface name
						strcpy(nif->_name, tmp->ifa_name);
					}
				}

				// Find the last interface
				if (!result) {
					sa = (struct sockaddr_in *) tmp->ifa_addr;
					
					if (!sa) {
						Error("Unknown error with sockaddr_in");
						result = 1;
					}
				}

				// Copy the ip address
				if (!result) {
					strcpy(nif->_ipaddr, inet_ntoa(sa->sin_addr));

					if (!strlen(nif->_ipaddr)) {
						result = 1;
						Error("Received an empty ip address");
					}
				}

				// Save into linked list
				if (!result) {
					if (nifRoot == 0) {
						nifRoot = nif;
					} else {
						// Find the last node
						struct NetInterface * nifTmp = 0;
						for (nifTmp = nifRoot; nifTmp != 0; nifTmp = nifTmp->_next) {
							if (nifTmp->_next == 0) { // Last one is when next==0
								nifTmp->_next = nif;
								break;
							}
						}
					}
				}
			}
		} while ((tmp = tmp->ifa_next) && !result);
	}

	if (!result) {
		if (nifRoot == 0) {
			Error("Could not find any interfaces");
			result = 1;
		}
	}

	// Now find the mac address for the current interfaces we have found
	if (!result) {
		tmp = addrs; // Reload
		
		do {
			bool cont = false;
			struct NetInterface * nif = 0;

			if (		tmp->ifa_addr  // Valid address
				&& 	(tmp->ifa_flags & IFF_UP) // The interface is active
				&& 	!(tmp->ifa_flags & IFF_LOOPBACK)) { // Interface is not a loopback
				if (tmp->ifa_addr->sa_family == AF_HW) {
					cont = true; // Found an interface with an IP address
				}
			}

			// Find the correct interface by comparing the interface names
			if (cont) {
				cont = false;

				// We assume nifRoot is nonnull
				for (nif = nifRoot; nif != 0; nif = nif->_next) {
					if (!strcmp(nif->_name, tmp->ifa_name)) {
						cont = true;
						break;
					}
				}
			} 

			if (cont) {
				if (!result) {
					char * mac = CopyMacAddress(tmp->ifa_addr, &result);

					if (!result) {
						strcpy(nif->_macaddr, mac);
					}

					free(mac);
				}
			}
		} while ((tmp = tmp->ifa_next) && !result);
	}

	// Print data
	if (!result) {
		for (struct NetInterface * nif = nifRoot; nif != 0; nif = nif->_next) {
			printf("Interface: %s\n", nif->_name);
			printf("\tIP: %s\n", nif->_ipaddr);
			printf("\tMAC: %s\n", nif->_macaddr);
		}
	}

	freeifaddrs(addrs);

	return result;
}

