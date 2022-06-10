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

#define HOSTNAME_BUF_LENGTH 200

/**
 * https://www.sanfoundry.com/c-program-get-ip-address/
 */
int GetIPAddressForHostname(const char * hostname, char * buf) {
	int result = 0;
	struct hostent * ent = gethostbyname(hostname);
	
	if (ent == 0) {
		result = 1;
		Error("Cannot get host by name");
	} else {
		char * s1 = inet_ntoa(*((struct in_addr *) ent->h_addr_list[0]));

		if (s1) {
			strcpy(buf, s1);
		} else {
			Error("Unknown return value");
			result = 1;
		}
	}

	return result;
}

int main(int argc, char * argv[]) {
	int result = 0;
	char hostname[HOSTNAME_BUF_LENGTH], ipaddress4[20];

	if (gethostname(hostname, HOSTNAME_BUF_LENGTH)) {
		result = 1;
		Error("Error getting hostname for device");
	} else if (GetIPAddressForHostname(hostname, ipaddress4)) {
		result = 1;
		Error("Error getting ip address for device");
	} else {
		printf("Hostname: %s\n", hostname);
		printf("IPv4: %s\n", ipaddress4);
	}

	return result;
}
