/**
 * author: Brando 
 * date: 5/1/2022
 *
 * Checklist:
 */

// INCLUDES 

#include <clib/clib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <libgen.h>

// CONSTANTS

/**
 * Holds the name of the script
 */
char SCRIPT_ARG[PATH_MAX];

/**
 * Allows user to show each path we find
 */
const char * VERBOSE_ARG = "-v";

// PROTOTYPES

/**
 * Prints the byteSize value into a format that represents
 * a comprehensible byte format
 */
int PrintSize(unsigned long long byteSize);

// FUNCTIONS

void Help() {
	printf("usage: %s [ %s ] <path>\n", SCRIPT_ARG, VERBOSE_ARG);

	printf("\nArguments:\n");
	printf("\t%s : Verbose mode\n", VERBOSE_ARG);
	printf("\t<path> : Can be absolute or relative.  Cannot be a symbolic link\n");
}

int main(int argc, char * argv[]) {
	int result = 0;
	char * buf = 0;
	unsigned long long size = 0;
	char path[PATH_MAX];
	unsigned char options = 0;

	// Get a copy of the script name 
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
			Help();
			result = 1;
		} else if (argc > 3) {
			Error("Too many arguments\n");
			Help();
			result = 1;
		} else {
			strcpy(path, argv[argc - 1]);

			if (!strlen(path)) {
				result = 1;
				Error("Arg 1 is empty");
				Help();
			}
		}
	}
	
	// Make sure the path the user provided exists
	if (!result) {
		if (!PathExists(path)) {
			Error("Path '%s' does not exist!", path);
			result = 1;
		}	
	}

	// See if user wants to show each path we find
	if (!result) {
		if (DoesStringArrayContain(argv, argc, "-v")) {
			options |= kCalculateSizeOptionsVerbose;
		}
	}

	// Check what type of path this is
	// and if depending of the type, we 
	// will calculate the total size of it
	if (!result) {
		if (IsSymbolicLink(path)) {
			result = 1;
			Error("Path '%s' is a symbolic link.  We cannot calculate size for sym links", path);
		} else if (IsFile(path)) {
			size = CalculateSizeFile(path, options, &result);
		} else if (IsDirectory(path)) {
			size = CalculateSizeDirectory(path, options, &result);
		} else {
			result = 1;
			Error("Unknown file type for '%s'", path);
		}
	}

	// Display size
	if (!result) {
		result = PrintSize(size);
	}
	
	return result;
}

int PrintSize(unsigned long long byteSize) {
	char unit[10];
	double value = 0.0, d1 = 0.0;
	long long byteLength = 0;
	if (GetByteStringRepresentation(byteSize, unit)) {
		Error("Error with getting byte representation");
	} else {
		printf("%s\n", unit);
	}

	return 0;
}

