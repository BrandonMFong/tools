/**
 * author: Brando
 * date: 6/2/22
 */

#include "../cutils/utilities.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>

#define HELP_ARG "-h"
#define DATE_ARG "-c"
#define MILT_ARG "-m"

/**
 * Holds the name of the script
 */
char SCRIPT_ARG[PATH_MAX];

void Help() {
	printf("usage: %s <args> [ %s ]\n", SCRIPT_ARG, HELP_ARG);

	printf("arguments:\n");
	printf("  %s : Prints today's date\n", DATE_ARG);
	printf("  %s : Prints military time\n", MILT_ARG);
}

/**
 * Sweeps the array to see of it contains the string specified by element
 */
bool DoesStringArrayContain(char ** strArr, int arrSize, const char * element) {
	for (int i = 0; i < arrSize; i++) {
		if (!strcmp(element, strArr[i])) {
			return true;
		}
	}

	return false;
}

int main(int argc, char * argv[]) {
	char * buf = basename(argv[0]);
	strcpy(SCRIPT_ARG, buf);

	if (DoesStringArrayContain(argv, argc, HELP_ARG)) {
		Help();
	} else {
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		int h, m, s;
		bool milTime = DoesStringArrayContain(argv, argc, MILT_ARG);

		if (milTime) {
			h = tm.tm_hour;
		} else {
			h = tm.tm_hour > 12 ? tm.tm_hour - 12 : tm.tm_hour;
		}

		m = tm.tm_min;
		s = tm.tm_sec;
		
		printf("%02d:%02d:%02d%s", 
			h,
			m, 
			s,
			milTime ? "" : (tm.tm_hour < 12 ? " AM" : " PM")
		);

		if (DoesStringArrayContain(argv, argc, DATE_ARG)) {
			printf(" %02d/%02d/%d",tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900); 
		}

		printf("\n");
	}

	return 0;
}

