/**
 * author: Brando 
 * date: 5/1/2022
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <libgen.h>

#define PATH_MAX 4096

char SCRIPT_ARG[PATH_MAX];
char PATH_ARG[PATH_MAX];

void help() {
	printf("usage: %s <path>\n", SCRIPT_ARG);
}

void error(const char * format, ...) {
	const int logSize = 1024;
	char logString[logSize];
	va_list args;
	
	va_start(args, format);
	vsnprintf(
		logString,
		logSize,
		format,
		args);

	va_end(args);

	printf("Error: %s\n", logString);
}

int main(int argc, char * argv[]) {
	int result = 0;
	char * buf = 0;

	buf = basename(argv[0]);
	
	strcpy(SCRIPT_ARG, buf);
	if (!strlen(SCRIPT_ARG)) {
		result = 1;
		error("There was a problemt with the first argument");
	}

	if (!result) {
		if (argc < 2) {
			error("Too few arguments\n");
			help();
			result = 1;
		} else if (argc > 2) {
			error("Too many arguments\n");
			help();
			result = 1;
		} else {
			strcpy(PATH_ARG, argv[1]);

			if (!strlen(PATH_ARG)) {
				result = 1;
				error("Arg 1 is empty");
				help();
			}
		}
	}
	
	return result;
}
