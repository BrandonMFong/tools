/**
 * author: Brando
 * date: 6/10/22
 */

#include <clib/clib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>

int main(void) {
	int result = 0;
	char buf[8192] = {0};
	struct ifconf ifc = {0};
	struct ifreq *ifr = NULL;
	int sock = 0;
	int nInterfaces = 0;
	int i = 0;
	char ip[INET6_ADDRSTRLEN] = {0};
	char macp[19];
	struct ifreq *item;
	struct sockaddr *addr;

	/* Get a socket handle. */
	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if(sock < 0) {
		result = 1;
		Error("Socket error");
	}

	if (!result) {
		/* Query available interfaces. */
		ifc.ifc_len = sizeof(buf);
		ifc.ifc_buf = buf;
		if(ioctl(sock, SIOCGIFCONF, &ifc) < 0) {
			result = 1;
			Error("ioctl error");
		}
	}

	if (!result) {
	 	/* Iterate through the list of interfaces. */
	 	ifr = ifc.ifc_req;
		nInterfaces = ifc.ifc_len / sizeof(struct ifreq);

		for(i = 0; (i < nInterfaces) && !result; i++) 
		{
			item = &ifr[i];

			addr = &(item->ifr_addr);

			/* Get the IP address*/
			if(ioctl(sock, SIOCGIFADDR, item) < 0) {
				Error("ioctl error with ip addr");
				result = 1;
			}

			if (!result) {
				if (inet_ntop(AF_INET, &(((struct sockaddr_in *)addr)->sin_addr), ip, sizeof ip) == NULL)
				    break;

				/* Get the MAC address */
				if(ioctl(sock, SIOCGIFHWADDR, item) < 0) {
				    result = 1;
				    Error("ioctl error with mac addr");
				}
			}

			/* display result */
			if (!result) {
				sprintf(macp, " %02x:%02x:%02x:%02x:%02x:%02x", 
				(unsigned char)item->ifr_hwaddr.sa_data[0],
				(unsigned char)item->ifr_hwaddr.sa_data[1],
				(unsigned char)item->ifr_hwaddr.sa_data[2],
				(unsigned char)item->ifr_hwaddr.sa_data[3],
				(unsigned char)item->ifr_hwaddr.sa_data[4],
				(unsigned char)item->ifr_hwaddr.sa_data[5]);

				printf("%s: %s %s \n", item->ifr_name, ip, macp);
			}
		}
	}

  return result;
}

