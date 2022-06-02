/**
 * author: Brando 
 * date: 6/1/2022
 *
 * Checklist:
 */

#include "../cutils/utilities.h"
#include <libgen.h>
#include <string.h>
#include <stdio.h>

const char SCRIPT_ARG[PATH_MAX];

void Help() {
	printf("usage: %s <path>\n", SCRIPT_ARG);	
}

int main(int argc, char * argv[]) {
	int result = 0;
	char path[PATH_MAX];
	strcpy((char *) SCRIPT_ARG, basename(argv[0]));

	if (argc < 2) {
		Help(); 
	} else {
		strcpy(path, argv[1]);
		
		if (!PathExists(path)) {
			result = 1;
			Error("Path '%s' does not exist", path);
		}
	}

	return 0;
}
