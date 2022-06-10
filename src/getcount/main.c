/**
 * author: Brando
 * date: 6/9/2022
 */

#include "../cutils/utilities.h"
#include <stdio.h>
#include <string.h>

char TOOL_ARG[PATH_MAX];

void Help() {
	printf("usage: %s <path>\n", TOOL_ARG);
}

int main(int argc, char * argv[]) {
	int result = 0;
	
	strcpy(TOOL_ARG, argv[0]);

	if (argc != 2) {
		Help();
	} else {

	}

	return result;
}
