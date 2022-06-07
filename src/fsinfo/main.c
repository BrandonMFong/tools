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
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

const char SCRIPT_ARG[PATH_MAX];

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

	if (!result) {
		result = PrintInfo(path);
	}

	return 0;
}

void SetDateStringForTime(char * outStr, const time_t * tm) {
	const char * dateFormat = "%02d:%02d:%02d%s %02d/%02d/%d";
	struct tm ct = *localtime(tm);

	sprintf(outStr, dateFormat,
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
	const char * dateFormat = "%02d:%02d:%02d%s %02d/%02d/%d";
	char absPath[PATH_MAX];
	int result = 0;
	char creation[strlen(dateFormat)], lastModified[strlen(dateFormat)];
	struct stat st;

	realpath(path, absPath);
	stat(path, &st);
	SetDateStringForTime(creation, &st.st_ctime);

	printf(	"Full path: %s\n"
		"Created: %s\n"
		"\n", absPath, creation);
	return result;
}

