#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

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
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	
	// example 
	//printf("now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	
	printf(
		"%02d:%02d:%02d %s", 
		tm.tm_hour > 13 ? tm.tm_hour - 12 : tm.tm_hour, 
		tm.tm_min, 
		tm.tm_sec,
		tm.tm_hour < 12 ? "AM" : "PM"
	);

	if (DoesStringArrayContain(argv, argc, "-c")) {
		printf(" %02d/%02d/%d",tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900); 
	}

	printf("\n");
}

