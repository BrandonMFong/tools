/**
 * author: Brando 
 * date: 6/2/22
 */

#include "clib.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <libgen.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>

unsigned long long CalculateSizeDirectory(const char * path, unsigned char options, int * err) {
	unsigned long long result = 0;
	int error = 0;
	DIR * dir = 0;
	struct dirent * sdirent = 0;
	char tempPath[PATH_MAX];
	char fullPath[PATH_MAX];
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
				// Construct the path
				sprintf(tempPath, "%s/%s", path, sdirent->d_name);
				
				// Get full path
				if (!realpath(tempPath, fullPath)) {
					error = 1;
				}

				if (!error) {
					size = 0;

					// If we are working with a directory, then we need 
					// too recursively call this function again 
					//
					// If path is not a diretory or a path then we will 
					// do nothing 
					//
					// We will also be ignoring symbolic links 
					if (IsSymbolicLink(fullPath)) {
						size = 0;
					} else if (IsDirectory(fullPath)) {
						size = CalculateSizeDirectory(fullPath, options, &error);
					
					// Otherwise, we will just get the size of this path 
					} else if (IsFile(fullPath)) {
						size = CalculateSizeFile(fullPath, options, &error);
					}

					if (!error) {
						result += size;
					}
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

unsigned long long CalculateSizeFile(const char * path, unsigned char options, int * error) {
       unsigned long long result = 0;
       struct stat buf;
       
       if (!stat(path, &buf)) {
               result = buf.st_size;

	       if (options & kCalculateSizeOptionsVerbose) {
			printf("(Bytes: %llu) %s\n", result, path);
	       }
       } else {
               if (error != 0) {
                       *error = 1;
                       Error("Error attempting to calculate file size for: %s", path);
               }
       }

       return result;
}

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

bool IsSymbolicLink(const char * path) {
	struct stat statbuf;

	if (lstat(path, &statbuf)) {
		return false;
	} else {
		return S_ISLNK(statbuf.st_mode);
	}
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

int GetByteStringRepresentation(unsigned long long byteSize, char * outStr) {
	double value = 0.0;
	char unit[10];

	if (outStr == 0) {
		return 1;
	} else {
		strcpy(outStr, ""); // init 

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

		sprintf(outStr, "%.2f %s", value, unit);
	}

	return 0;
}

bool DoesStringArrayContain(char ** strArr, int arrSize, const char * element) {
	for (int i = 0; i < arrSize; i++) {
		if (!strcmp(element, strArr[i])) {
			return true;
		}
	}

	return false;
}

char * CopyString(const char * string, int * err) {
	int error = 0;
	unsigned long long size = strlen(string);

	char * result = (char *) malloc(size + 1);

	if (result) {
		memset(result, 0, size + 1);
		strcpy(result, string);
	} else {
		error = 1;
	}

	if (err != 0) *err = error;

	return result;
}

int ResolveHostname(const char * hostname, char * ip) {
	int result = 0;
	struct hostent * he;
	struct in_addr ** addrList;

	if (!(he = gethostbyname(hostname))) {
		result = 1;
	}

	if (!result) {
		addrList = (struct in_addr **) he->h_addr_list;

		if (addrList == 0) {
			result = 2;
		}
	}
	
	if (!result) {
		if (addrList[0] == 0) {
			result = 3;
		} else{
			strcpy(ip, inet_ntoa(*addrList[0]));
		}
	}
	
	return result;
}

