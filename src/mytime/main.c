/**
 * author: Brando
 * date: 6/2/22
 */

#include <bflibc/bflibc.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>

#ifdef LINUX
#include <linux/limits.h>
#endif

#define HELP_ARG "-h"
#define MILT_ARG "-m"
#define LOCK_ARG "-l"
#define NS_ARG "-ns"

/**
 * Holds the name of the script
 */
char SCRIPT_ARG[PATH_MAX];
bool SHOW_MIL_TIME = false;
bool SHOW_NANO_SECS = true;

void Help() {
	printf("usage: %s <args> [ %s ]\n", SCRIPT_ARG, HELP_ARG);

	printf("arguments:\n");
	printf("  %s : Prints military time\n", MILT_ARG);
	printf("  %s : Keeps printing time until user cancels program\n", LOCK_ARG);
	printf("  %s : Prints out time to the nanosecond\n", NS_ARG);
}

void PrintTime() {
	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);
	struct tm tm = *localtime(&spec.tv_sec);
	int h, m, s;

	if (SHOW_MIL_TIME) {
		h = tm.tm_hour;
	} else {
		h = tm.tm_hour > 12 ? tm.tm_hour - 12 : tm.tm_hour;
	}

	m = tm.tm_min;
	s = tm.tm_sec;
	
	printf("%02d:%02d:%02d", 
		h,
		m, 
		s
	);

	if (SHOW_NANO_SECS) {
		int ms = spec.tv_nsec * pow(10, -6);
		int us = (spec.tv_nsec * pow(10, -3)) - (ms * pow(10, 3));
		int ns = spec.tv_nsec - ((ms * pow(10, 6)) + (us * pow(10, 3)));

		printf(".%03d.%03d.%03d", ms, us, ns);
	}

	printf(" %s", SHOW_MIL_TIME ? "" : (tm.tm_hour < 12 ? "AM" : "PM"));

	printf(", %02d/%02d/%d", tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900); 
}

int main(int argc, char * argv[]) {
	char * buf = basename(argv[0]);
	strcpy(SCRIPT_ARG, buf);

	if (BFArrayStringContainsString(argv, argc, HELP_ARG)) {
		Help();
	} else {
		SHOW_MIL_TIME = BFArrayStringContainsString(argv, argc, MILT_ARG);
		SHOW_NANO_SECS = BFArrayStringContainsString(argv, argc, NS_ARG);
		bool lock = BFArrayStringContainsString(argv, argc, LOCK_ARG);
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

