/**
 * author: Brando
 * date: 7/19/22
 */

#include <clib/clib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char TOOL_ARG[PATH_MAX];

void Help() {
	printf("usage: %s <relative path>\n", TOOL_ARG);
}

int main(int argc, char * argv[]) {
	strcpy(TOOL_ARG, argv[0]);

	if (argc != 2) {
		Help();
	} else {
		char buf[PATH_MAX];
		realpath(argv[1], buf);
		printf("%s\n", buf);
	}
	
	return 0;
}
