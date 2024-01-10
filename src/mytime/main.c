/**
 * author: Brando
 * date: 6/2/22
 */

#include <bflibc/bflibc.h>
#include <stdio.h>
#include <stdlib.h>
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
#define US_ARG "-u"
#define EPOCH_ARG "-e"
#define BRIEF_DESCRIPTION_ARG "--brief-description"

/**
 * Holds the name of the script
 */
char SCRIPT_ARG[PATH_MAX];
bool SHOW_MIL_TIME = false;
bool SHOW_MICRO_SECS = false;

void Help() {
	printf("usage: %s <args> [ %s ]\n", SCRIPT_ARG, HELP_ARG);

	printf("arguments:\n");
	printf("  %s : Prints military time\n", MILT_ARG);
	printf("  %s : Keeps printing time until user cancels program\n", LOCK_ARG);
	printf("  %s : Prints out time to the microsecond\n", US_ARG);
	printf(	"  %s [ <epoch value> ] : Prints out current epoch\n"
			"     time.  If value is passed, will print out the\n"
			"     date time version of epoch time\n", EPOCH_ARG);
}

void BriefDescription() {
	printf("time tool. Can print time and handle epoch time conversions\n");
}

void PrintTime(const time_t sec, const long nsec) {
	struct tm tm = *localtime(&sec);
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

	if (SHOW_MICRO_SECS) {
		int ms = nsec * pow(10, -6);
		int us = (nsec * pow(10, -3)) - (ms * pow(10, 3));

		printf(".%03d%03d", ms, us);
	}

	printf(" %s", SHOW_MIL_TIME ? "" : (tm.tm_hour < 12 ? "AM" : "PM"));

	printf(", %02d/%02d/%d", tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900); 
}

int main(int argc, char * argv[]) {
	int error = 0;
	char * buf = basename(argv[0]);
	strcpy(SCRIPT_ARG, buf);
	bool lock = false;
	bool help = false;
	time_t inputEpoch = 0;
	bool epochFlag = false;
	bool brieflyDescribe = false;

	for (int i = 0; i < argc; i++) {
		if (!strcmp(argv[i], HELP_ARG)) {
			help = true;
		} else if (!strcmp(argv[i], BRIEF_DESCRIPTION_ARG)) {
			brieflyDescribe = true;
		} else if (!strcmp(argv[i], MILT_ARG)) {
			SHOW_MIL_TIME = true;
		} else if (!strcmp(argv[i], EPOCH_ARG)) {
			epochFlag = true;
			if ((i + 1) < argc) {
				inputEpoch = (time_t) atof(argv[i + 1]);
			}
		} else if (!strcmp(argv[i], US_ARG)) {
			SHOW_MICRO_SECS = true;
		} else if (!strcmp(argv[i], LOCK_ARG)) {
			lock = true;
		}
	}

	if (brieflyDescribe) {
		BriefDescription();
	} else if (help) {
		Help();
	} else if (error == 0) {
		if (epochFlag) {
			if (inputEpoch) {
				PrintTime(inputEpoch, 0);
				printf("\n");
			} else {
				printf("%.2f\n", BFTimeGetCurrentTime());
			}
		} else { 
			do {
				struct timespec spec;
				clock_gettime(CLOCK_REALTIME, &spec);

				PrintTime(spec.tv_sec, spec.tv_nsec);

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
	}

	return error;
}

