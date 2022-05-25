/**
 * author: Brando 
 * date: 5/1/2022
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <libgen.h>
#include <stdbool.h>
#include <sys/stat.h>

#define PATH_MAX 4096

char SCRIPT_ARG[PATH_MAX];
char PATH_ARG[PATH_MAX];

/**
 * Prints std help
 */
void Help() {
	printf("usage: %s <path>\n", SCRIPT_ARG);
}

/**
 * Prints std error
 */
void Error(const char * format, ...) {
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

/**
 * Tests if path exists
 */
bool PathExists(const char * path) {
	struct stat buffer;
	return (stat(path, &buffer) == 0);
}

int main(int argc, char * argv[]) {
	int result = 0;
	char * buf = 0;

	buf = basename(argv[0]);
	
	strcpy(SCRIPT_ARG, buf);
	if (!strlen(SCRIPT_ARG)) {
		result = 1;
		Error("There was a problemt with the first argument");
	}
	
	// Get the second argument
	// 
	// Should be the path to a file
	if (!result) {
		if (argc < 2) {
			Error("Too few arguments\n");
			Help();
			result = 1;
		} else if (argc > 2) {
			Error("Too many arguments\n");
			Help();
			result = 1;
		} else {
			strcpy(PATH_ARG, argv[1]);

			if (!strlen(PATH_ARG)) {
				result = 1;
				Error("Arg 1 is empty");
				Help();
			}
		}
	}
	
	// Make sure the path the user provided exists
	if (!result) {
		if (!PathExists(PATH_ARG)) {
			Error("Path '%s' does not exist!", PATH_ARG);
		}	
	}

	if (!result) {

	}
	
	return result;
}
