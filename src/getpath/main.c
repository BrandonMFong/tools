/**
 * author: Brando
 * date: 7/19/22
 */

#include <bflibc/bflibc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifdef LINUX
#include <linux/limits.h>
#endif

const char * BRIEF_DESCRIPTION_ARG = "--brief-description";
char TOOL_ARG[PATH_MAX];

void Help() {
	printf("usage: %s <relative path>\n", TOOL_ARG);
}

void BriefDescription() {
	printf("prints out absolute path\n");
}

int main(int argc, char * argv[]) {
	strcpy(TOOL_ARG, argv[0]);

	if (argc < 2) {
		Help();
	} else {
		if (!strcmp(argv[1], BRIEF_DESCRIPTION_ARG)) {
			BriefDescription();
		} else {
			char buf[PATH_MAX];
			realpath(argv[1], buf);
			printf("%s\n", buf);
		}
	}
	
	return 0;
}

