#include <stdio.h>
#include <time.h>
#include <stdint.h>

int main() {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	
	// example 
	//printf("now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	
	
	printf(
		"%02d:%02d:%02d %s\n", 
		tm.tm_hour > 13 ? tm.tm_hour - 12 : tm.tm_hour, 
		tm.tm_min, 
		tm.tm_sec,
		tm.tm_hour < 12 ? "AM" : "PM"
	);
}

