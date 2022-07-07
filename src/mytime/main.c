/**
 * author: Brando
 * date: 6/2/22
 */

#include <clib/clib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>
#include <unistd.h>

#define HELP_ARG "-h"
#define MILT_ARG "-m"
#define LOCK_ARG "-l"

/**
 * Holds the name of the script
 */
char SCRIPT_ARG[PATH_MAX];
bool SHOW_MIL_TIME = false;

void Help() {
	printf("usage: %s <args> [ %s ]\n", SCRIPT_ARG, HELP_ARG);

	printf("arguments:\n");
	printf("  %s : Prints military time\n", MILT_ARG);
	printf("  %s : Keeps printing time until user cancels program\n", LOCK_ARG);
}

void PrintTime() {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	int h, m, s;

	if (SHOW_MIL_TIME) {
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
		SHOW_MIL_TIME ? "" : (tm.tm_hour < 12 ? " AM" : " PM")
	);

	printf(", %02d/%02d/%d",tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900); 
}

int main(int argc, char * argv[]) {
	char * buf = basename(argv[0]);
	strcpy(SCRIPT_ARG, buf);

	if (DoesStringArrayContain(argv, argc, HELP_ARG)) {
		Help();
	} else {
		SHOW_MIL_TIME = DoesStringArrayContain(argv, argc, MILT_ARG);
		bool lock = DoesStringArrayContain(argv, argc, LOCK_ARG);
		do {
			PrintTime();

			if (lock) {
				fflush(stdout);
				sleep(1);
				printf("\r");
			} else {
				printf("\n");
				break;
			}
		} while (1);
	}

	return 0;
}

