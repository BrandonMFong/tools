/**
 * author: Brando 
 * date: 5/1/2022
 *
 * Checklist:
 * 	[] Do file 
 * 	[] Do directory
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <libgen.h>
#include <stdbool.h>
#include <sys/stat.h>

// MACROS

#define PATH_MAX 4096

#define KILOBYTE 1024
#define MEGABYTE KILOBYTE * 1024
#define GIGABYTE MEGABYTE * 1024
#define TERABYTE GIGABYTE * 1024

// CONSTANTS

char SCRIPT_ARG[PATH_MAX];

// PROTOTYPES

int CalculateDirectorySize(int * error);

/**
 * Calculates the file size of the path  
 */
unsigned long long CalculateFileSize(const char * path, int * error);

/**
 * Prints the byteSize value into a format that represents
 * a comprehensible byte format
 */
int PrintSize(unsigned long long byteSize);

// FUNCTIONS

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

bool IsFile(const char * path) {
	struct stat buf;

	if (stat(path, &buf)) {
		return false;
	} else {
		return S_ISREG(buf.st_mode);
	}
}

bool IsDirectory(const char * path) {
	struct stat buf;

	if (stat(path, &buf)) {
		return false;
	} else {
		return S_ISDIR(buf.st_mode);
	}
}

int main(int argc, char * argv[]) {
	int result = 0;
	char * buf = 0;
	unsigned long long size = 0;
	char path[PATH_MAX];

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
			strcpy(path, argv[1]);

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
		}	
	}

	// Check what type of path this is
	if (!result) {
		if (IsFile(path)) {
			printf("Path is a file\n");
			size = CalculateFileSize(path, &result);
		} else if (IsDirectory(path)) {
			printf("Path is a directory\n");
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

unsigned long long CalculateFileSize(const char * path, int * error) {
	unsigned long long result = 0;
	struct stat buf;
	
	if (!stat(path, &buf)) {
		result = buf.st_size;
	} else {
		if (error != 0) {
			*error = 1;
			Error("Error attempting to calculate file size for: %s", path);
		}
	}

	return result;
}

int PrintSize(unsigned long long byteSize) {
	char unit[10];
	double value = 0.0, d1 = 0.0;

	strcpy(unit, ""); // init 

	// MegaByte
	if (byteSize > MEGABYTE) {
		value = byteSize / MEGABYTE;
		d1 = (byteSize % MEGABYTE);

		value += (d1 / MEGABYTE);
		strcpy(unit, "gb");
	// KiloByte
	} else if (byteSize > KILOBYTE) {
		value = byteSize / KILOBYTE;
		d1 = (byteSize % KILOBYTE);

		value += (d1 / KILOBYTE);
		strcpy(unit, "kb");
	// Byte
	} else {
		value = byteSize;
		if (value == 1) {
			strcpy(unit, "byte");
		} else {
			strcpy(unit, "bytes");
		}
	}
	
	printf("%.2f %s\n", value, unit);
	return 0;
}

