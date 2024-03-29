/**
 * author: Brando
 * date: 6/22/22
 */

#include <bflibc/bflibc.h>
#include <ctools/tools.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>

#ifdef LINUX
#include <linux/limits.h>
#endif

char TOOL_ARG[PATH_MAX];
const char * BRIEF_DESCRIPTION_ARG = "--brief-description";

void Help() {
	printf("usage: %s <domain>\n", TOOL_ARG);
}

void BriefDescription() {
	printf("returns ip address for domain\n");
}

int TOOL_MAIN(int argc, char * argv[]) {
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
		if (!strcmp(argv[1], BRIEF_DESCRIPTION_ARG)) {
			result = 1;
			BriefDescription();
		} else {
			domain = argv[argc - 1];

			if (domain == 0) {
				result = 1;
				BFErrorPrint("Null domain");
				Help();
			} else if (strlen(domain) == 0) {
				BFErrorPrint("Empty domain");
				result = 1;
			} else {
				result = BFNetGetIPForHostname(domain, ip);
			}
		}
	}

	if (!result) {
		printf("%s\n", ip);
	}

	return result;
}

#ifdef TESTING

int TOOL_TEST(int argc, char ** argv) {
	printf("testing %s\n", argv[0]);
	return 0;
}

#endif // TESTING

