/**
 * author: Brando 
 * date: 6/2/22
 */

#include "utilities.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <libgen.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>

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

