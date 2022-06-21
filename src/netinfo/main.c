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

#ifdef LINUX
#define AF_HW AF_PACKET
#elif OSX
#define AF_HW AF_LINK
#else 
#error Unknown OS
#endif

int main() {
	struct ifaddrs *addrs,*tmp;
	struct sockaddr_in *sa;

	getifaddrs(&addrs);
	tmp = addrs;

	while (tmp)
	{
		// https://stackoverflow.com/questions/4139405/how-can-i-get-to-know-the-ip-address-for-interfaces-in-c
		if (tmp->ifa_addr) {
			if (tmp->ifa_addr->sa_family == AF_INET) {
				printf("%d: %s %d\n", tmp->ifa_addr->sa_family, tmp->ifa_name, sizeof(tmp->ifa_addr->sa_data));

				sa = (struct sockaddr_in *) tmp->ifa_addr;
				char * addr = inet_ntoa(sa->sin_addr);
				printf("%s\n", addr);
			} else if (tmp->ifa_addr->sa_family == AF_HW) {
				printf("%d: %s %d\n", tmp->ifa_addr->sa_family, tmp->ifa_name, sizeof(tmp->ifa_addr->sa_data));
				for (int i = 0; i < sizeof(tmp->ifa_addr->sa_data); i++) {
					printf("%02x ", (unsigned char) tmp->ifa_addr->sa_data[i]);
				}

				printf("\n");
			}
		}

		tmp = tmp->ifa_next;
	}

	freeifaddrs(addrs);
	return 0;
}

