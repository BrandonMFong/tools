/**
 * author: Brando
 * date: 6/22/22
 */

#include <clib/clib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

char TOOL_ARG[PATH_MAX];

void Help() {
	printf("usage: %s <domain>\n", TOOL_ARG);
}

int main(int argc, char * argv[]) {
	int result = 0;
	char * buf = 0;
	char ip[(3 * 4) + 4];
	const char * domain = 0;

	// Get a copy of the script name 
	buf = basename(argv[0]);
	strcpy((char *) TOOL_ARG, buf);

	if (argc != 2) {
		Help();
		result = 1;
	} else {
		domain = argv[argc - 1];

		if (domain == 0) {
			result = 1;
			Error("Null domain");
			Help();
		} else if (strlen(domain) == 0) {
			Error("Empty domain");
			result = 1;
		} else {
			result = ResolveHostname(domain, ip);
		}
	}

	if (!result) {
		printf("%s\n", ip);
	}

	return result;
}

