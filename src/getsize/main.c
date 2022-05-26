/**
 * author: Brando 
 * date: 5/1/2022
 *
 * Checklist:
 */

// INCLUDES 

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <libgen.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>

// MACROS

#define PATH_MAX 4096

#define KILOBYTE 1024
#define MEGABYTE (long long) (KILOBYTE * 1024)
#define GIGABYTE (long long) (MEGABYTE * 1024)
#define TERABYTE (long long) (GIGABYTE * 1024)

// CONSTANTS

/**
 * Holds the name of the script
 */
char SCRIPT_ARG[PATH_MAX];

// PROTOTYPES

/**
 * Calculates the directory size recursively
 */
unsigned long long CalculateDirectorySize(const char * path, int * error);

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

/**
 * Tests if path is a file
 */
bool IsFile(const char * path) {
	struct stat buf;

	if (stat(path, &buf)) {
		return false;
	} else {
		return S_ISREG(buf.st_mode);
	}
}

/**
 * Tests if path is a directory
 */
bool IsDirectory(const char * path) {
	struct stat buf;

	if (stat(path, &buf)) {
		return false;
	} else {
		return S_ISDIR(buf.st_mode);
	}
}

/**
 * Tests if the path is a symbolic link 
 */
bool IsSymbolicLink(const char * path) {
	struct stat statbuf;

	if (lstat(path, &statbuf)) {
		return false;
	} else {
		return S_ISLNK(statbuf.st_mode);
	}
}

int main(int argc, char * argv[]) {
	int result = 0;
	char * buf = 0;
	unsigned long long size = 0;
	char path[PATH_MAX];

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
			result = 1;
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
			size = CalculateFileSize(path, &result);
		} else if (IsDirectory(path)) {
			size = CalculateDirectorySize(path, &result);
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

unsigned long long CalculateDirectorySize(const char * path, int * err) {
	unsigned long long result = 0;
	int error = 0;
	DIR * dir = 0;
	struct dirent * sdirent = 0;
	char tempPath[PATH_MAX];
	unsigned long long size = 0;

	dir = opendir(path);
	
	if (dir == 0) {
		error = 1;
		Error("Could not open directory: '%s'", path);
	} else {
		// Go through each item in this directory
		while ((sdirent = readdir(dir)) && !error) {
			// We do not want to calculate the size of parent or current dir
			if (strcmp(sdirent->d_name, "..") && strcmp(sdirent->d_name, ".")) {
				// We want the full directory path
				sprintf(tempPath, "%s/%s", path, sdirent->d_name);

				size = 0;

				// If we are working with a directory, then we need 
				// too recursively call this function again 
				//
				// If path is not a diretory or a path then we will 
				// do nothing 
				//
				// We will also be ignoring symbolic links 
				if (IsSymbolicLink(tempPath)) {
					size = 0;
				} else if (IsDirectory(tempPath)) {
					size = CalculateDirectorySize(tempPath, &error);
				
				// Otherwise, we will just get the size of this path 
				} else if (IsFile(tempPath)) {
					size = CalculateFileSize(tempPath, &error);
				}

				if (!error) {
					result += size;
				}
			}
		}

		if (closedir(dir)) {
			error = !error ? 1 : error; 
			Error("Could not close directory: '%s'", path);
		}
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

/**
 * Converts the value to the level specified by scale
 */
double ConvertValueToScale(unsigned long long value, long long scale) {
	double result = 0, d1 = 0;

	result = value / scale;
	d1 = (value % scale);

	result += (d1 / scale);

	return result;
}

int PrintSize(unsigned long long byteSize) {
	char unit[10];
	double value = 0.0, d1 = 0.0;
	long long byteLength = 0;

	strcpy(unit, ""); // init 

	// Byte
	// 
	// Default
	value = byteSize;
	strcpy(unit, "b");

	// TeraByte
	if (byteSize > TERABYTE) {
		value = ConvertValueToScale(byteSize, TERABYTE);
		strcpy(unit, "tb");
	// GigaByte
	} else if (byteSize > GIGABYTE) {
		value = ConvertValueToScale(byteSize, GIGABYTE);
		strcpy(unit, "gb");
	// MegaByte
	} else if (byteSize > MEGABYTE) {
		value = ConvertValueToScale(byteSize, MEGABYTE);
		strcpy(unit, "mb");
	// KiloByte
	} else if (byteSize > KILOBYTE) {
		value = ConvertValueToScale(byteSize, KILOBYTE);
		strcpy(unit, "kb");
	}
	
	printf("%.2f %s\n", value, unit);
	return 0;
}

