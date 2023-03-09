/**
 * author: Brando 
 * date: 6/1/2022
 *
 * Checklist:
 */

#include <bflibc/bflibc.h>
#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <limits.h>

#ifdef LINUX
#include <linux/limits.h>
#endif

#define DATE_FORMAT "%02d:%02d:%02d%s %02d/%02d/%d"

char SCRIPT_ARG[PATH_MAX];

void Help() {
	printf("usage: %s <path>\n", SCRIPT_ARG);	
}

/**
 * Prints the info of path
 */
int PrintInfo(const char * path);

int main(int argc, char * argv[]) {
	int result = 0;
	char path[PATH_MAX];
	strcpy(SCRIPT_ARG, basename(argv[0]));

	if (argc < 2) {
		Help(); 
	} else {
		strcpy(path, argv[1]);
		
		if (!BFFileSystemPathExists(path)) {
			result = 1;
			BFErrorPrint("Path '%s' does not exist", path);
		}
	}

	if (!result) {
		result = PrintInfo(path);
	}

	return 0;
}

void SetDateStringForTime(char * outStr, const time_t * tm) {
	struct tm ct = *localtime(tm);

	sprintf(outStr, DATE_FORMAT,
		ct.tm_hour > 12 ? ct.tm_hour - 12 : ct.tm_hour,
		ct.tm_min,
		ct.tm_sec,
		ct.tm_hour < 12 ? " AM" : " PM",
		ct.tm_mon + 1,
		ct.tm_mday,
		ct.tm_year + 1900
	);
}

int PrintInfo(const char * path) {
	char absPath[PATH_MAX];
	int result = 0;
	char lastModified[strlen(DATE_FORMAT)];
	struct stat st;
	char unit[10];
	unsigned long long size = 0;
	long long available = -1;
	char availableUnit[10];

	if (realpath(path, absPath) == 0) {
		result = 1;	
	} 

	if (!result) {
		if (stat(path, &st)) {
			result = 1;
		}
	}

	if (!result) {
		if (BFFileSystemPathIsDirectory(absPath)) {
			size = BFFileSystemDirectoryGetSizeUsed(absPath, 0, &result);
		} else if (BFFileSystemPathIsFile(absPath)) {
			size = BFFileSystemFileGetSizeUsed(absPath, 0, &result);
		}
	}

	if (!result) {
		result = BFByteGetString(size, unit);
	}
	
	if (!result) {
		if (BFFileSystemPathIsDirectory(absPath)) {
			available = BFFileSystemPathGetSizeAvailable(absPath, &result);
		}
	}
	
	if (!result) {
		if (available == -1) {
			strcpy(availableUnit, "N/A");
		} else {
			result = BFByteGetString(available, availableUnit);
		}
	}

	if (!result) {
		SetDateStringForTime(lastModified, &st.st_mtime);
		printf(	"Full path: %s\n"
			"Size: %s\n"
			"Available: %s\n"
			"Last modified: %s\n"
			"\n", absPath, unit, availableUnit, lastModified);
	}

	return result;
}

