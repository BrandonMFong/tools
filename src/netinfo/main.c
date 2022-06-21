/**
 * author: Brando
 * date: 6/10/22
 */

#include <clib/clib.h>
#include <stdio.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/socket.h>

int main() {
struct ifaddrs *addrs,*tmp;

getifaddrs(&addrs);
tmp = addrs;

while (tmp)
{
     if (tmp->ifa_addr && (tmp->ifa_addr->sa_family == AF_INET))
        printf("%d: %s\n", tmp->ifa_addr->sa_family, tmp->ifa_name);

    tmp = tmp->ifa_next;
}

freeifaddrs(addrs);
	return 0;
}

