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

/**
 * Creates a copy of the mac address
 *
 * Caller must free
 */
char * CopyMacAddress(struct sockaddr * sa, int * err) {
	char * result = 0;
	int error = 0;
	char buf[20]; // buffer for the mac address string

	memset(&buf[0], 0, 20);

#ifdef LINUX
	struct sockaddr_ll * saddr = (struct sockaddr_ll *) sa;

	int l = 0;
	for (int i = 0; i < 6; i++) {
		l += sprintf(buf+l, "%02X%s", saddr->sll_addr[i], i < 5 ? ":" : "");
	}
#elif OSX
	unsigned char * t = (unsigned char *) LLADDR((struct sockaddr_dl *) sa);

	if (t) {
		int l = 0;
		for (int i = 0; i < 6; i++) {
			l += sprintf(buf+l, "%02X%s", t[i], i < 5 ? ":" : "");
		}
	} else {
		error = 1;
		Error("Error with LLADDR");
	}
#endif

	if (!error) {
		result = CopyString(buf, &error);
	}

	if (err != 0) {
		*err = error;
	}

	return result;
}

int main() {
	struct ifaddrs *addrs,*tmp;
	struct sockaddr_in *sa;

	getifaddrs(&addrs);
	tmp = addrs;

	while (tmp)
	{
		// https://stackoverflow.com/questions/4139405/how-can-i-get-to-know-the-ip-address-for-interfaces-in-c
		if (tmp->ifa_addr && (tmp->ifa_flags & IFF_UP) && !(tmp->ifa_flags & IFF_LOOPBACK)) {
			if (tmp->ifa_addr->sa_family == AF_INET) {
				printf("IP Address %d: %s %d\n", tmp->ifa_addr->sa_family, tmp->ifa_name, sizeof(tmp->ifa_addr->sa_data));

				sa = (struct sockaddr_in *) tmp->ifa_addr;
				char * addr = inet_ntoa(sa->sin_addr);
				printf("%s\n", addr);
			} else if (tmp->ifa_addr->sa_family == AF_HW) {
				printf("MAC Address %d: %s %d\n", tmp->ifa_addr->sa_family, tmp->ifa_name, sizeof(tmp->ifa_addr->sa_data));
				char * mac = CopyMacAddress(tmp->ifa_addr, 0);
				printf("%s\n", mac);
				free(mac);
			}
		}

		tmp = tmp->ifa_next;
	}

	freeifaddrs(addrs);
	return 0;
}

