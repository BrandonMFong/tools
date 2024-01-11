/**
 * author: Brando
 * date: 7/6/22
 */

#include <bflibc/bflibc.h>
#include <ctools/tools.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>

#ifdef LINUX
#include <linux/limits.h>
#endif

char TOOL_ARG[PATH_MAX];
const char * HELP_ARG = "--help";
const char * NUMERAL_ARG = "--num";
const char * UPPERCASE_ARG = "--upper";
const char * LOWERCASE_ARG = "--lower";
const char * SPECIAL_ARG = "--special";
const char * LENGTH_ARG = "-length";
const char * BRIEF_DESCRIPTION_ARG = "--brief-description";

/// Characters
const char * UPPERCASE_CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char * LOWERCASE_CHARACTERS = "abcdefghijklmnopqrstuvwxyz";
const char * NUMERAL_CHARACTERS = "1234567890";
const char * SPECIAL_CHARACTERS = "!#$%&'()*+,-./:;<=>?@[]^_`{|}~";

#define kCreateRandomStringOptionDefaultLength 16

/// Options
#define kCreateRandomStringOptionUseNumber 		(0x01 << 11)
#define kCreateRandomStringOptionUseLowercase 		(0x01 << 10)
#define kCreateRandomStringOptionUseUppercase 		(0x01 << 9)
#define kCreateRandomStringOptionUseSpecialCharacters 	(0x01 << 8)
#define kCreateRandomStringOptionLength 		(0xFF << 0)

#define kCreateRandomStringOptionDefault \
	/* Use numbers */ 	kCreateRandomStringOptionUseNumber | \
	/* Use lowercase */ 	kCreateRandomStringOptionUseLowercase | \
	/* Use uppercase */ 	kCreateRandomStringOptionUseUppercase | \
	/* Use special */ 	kCreateRandomStringOptionUseSpecialCharacters | \
       /* 16 character length*/	(kCreateRandomStringOptionLength & kCreateRandomStringOptionDefaultLength)

void Help() {
	printf(
		"usage: %s [ %s ] [ %s ] [ %s ] [ %s ] [ %s <num> ] [ %s ]\n", 
		TOOL_ARG, 
		UPPERCASE_ARG, 
		LOWERCASE_ARG, 
		NUMERAL_ARG, 
		SPECIAL_ARG,
	       	LENGTH_ARG,	
		HELP_ARG
	);

	printf("\nArguments:\n");
	printf("\t%s <number> : Number from 1 %d\n", LENGTH_ARG, 0xff);
	printf("\t%s : use \"%s\"\n", UPPERCASE_ARG, UPPERCASE_CHARACTERS);
	printf("\t%s : use \"%s\"\n", LOWERCASE_ARG, LOWERCASE_CHARACTERS);
	printf("\t%s : use \"%s\"\n", NUMERAL_ARG, NUMERAL_CHARACTERS);
	printf("\t%s : use \"%s\"\n", SPECIAL_ARG, SPECIAL_CHARACTERS);

	printf("\nDescription:\n");
	printf("\tAll characters are used by default\n");
}

void BriefDescription() {
	printf("generates random password\n");
}

/**
 * Creates a string copy of the password specified by options
 *
 * options bit specification:
 *    0-7: size of string
 * 	8: Use special characters
 * 	9: Use uppercase characters
 *     10: Use lowercase characters
 *     11: Use numbers
 *     12: reserved
 *     13: reserved
 *     14: reserved
 *     15: reserved
 */
char * CreateRandomString(unsigned short  options, int * err);

int main(int argc, char * argv[]) {
	int result = 0;
	unsigned short options = kCreateRandomStringOptionDefault;

	strcpy(TOOL_ARG, argv[0]);

	if (BFArrayStringContainsString(argv, argc, HELP_ARG)) {
		Help();
	} else if (BFArrayStringContainsString(argv, argc, BRIEF_DESCRIPTION_ARG)) {
		BriefDescription();
	} else {
		// If user specified options, then we must reset options to 0 so that we 
		// start with a fresh state
		if (argc > 1) {
			options = 0x0000;

			// character options
			if (BFArrayStringContainsString(argv, argc, UPPERCASE_ARG)) {
				options |= kCreateRandomStringOptionUseUppercase;
			}

			if (BFArrayStringContainsString(argv, argc, LOWERCASE_ARG)) {
				options |= kCreateRandomStringOptionUseLowercase;
			}

			if (BFArrayStringContainsString(argv, argc, NUMERAL_ARG)) {
				options |= kCreateRandomStringOptionUseNumber;
			}

			if (BFArrayStringContainsString(argv, argc, SPECIAL_ARG)) {
				options |= kCreateRandomStringOptionUseSpecialCharacters;
			}

			// If we did not get any options yet, reset the default options for the characters
			if (!options) {
				options = kCreateRandomStringOptionDefault & ~kCreateRandomStringOptionLength;
			}

			// Now get the length option
			int index = BFArrayStringGetIndexForString(argv, argc, LENGTH_ARG);

			// If length was not specified, then we will use the default
			// value of 16
			if (index == -1) {
				options |= kCreateRandomStringOptionDefaultLength;

			// Otherwise we must find the length
			} else {
				unsigned short length = 0;
				if (index + 1 < argc) {
					char * arg = argv[index + 1];
					length = (unsigned short) atoi(arg);
				} else {
					result = 1;
					BFErrorPrint("Indexing error with arguments");
				}

				if (!result) {
					if (length == 0) {
						result = 1;
						BFErrorPrint("User specified 0 length.  Please pass a nonzero number");
					} else {
						options |= length;
					}
				}
			}
		}

		if (!result) {
			char * password = CreateRandomString(options, &result);

			if (!result) {
				printf("%s\n", password);

				free(password);
			}
		}
	}

	return result;
}

char * CreateRandomString(unsigned short options, int * err) {
	char * result = 0;
	int error = 0;
	bool numbers = (options & kCreateRandomStringOptionUseNumber);
	bool lowercase = (options & kCreateRandomStringOptionUseLowercase);
	bool uppercase = (options & kCreateRandomStringOptionUseUppercase);
	bool special = (options & kCreateRandomStringOptionUseSpecialCharacters);
	unsigned char size = (options & kCreateRandomStringOptionLength);
	int bufSize = strlen(UPPERCASE_CHARACTERS) 
		+ strlen(LOWERCASE_CHARACTERS) 
		+ strlen(NUMERAL_CHARACTERS) 
		+ strlen(SPECIAL_CHARACTERS) 
		+ 1;
	char charBuf[bufSize];

	// Initialize variables and random lib
	memset(&charBuf[0], 0, bufSize);
	srand(time(0));

	result = (char *) malloc(size + 1);

	if (!result) {
		error = 1;
		BFErrorPrint("Could not allocate memory");
	}

	if (!error) {
		// read flags
		// before reading, the charBuf should be length 0.
		// If by the end of these if statements the charBuf
		// has no content, no character specification was set
		if (numbers) strcat(charBuf, NUMERAL_CHARACTERS);
		if (lowercase) strcat(charBuf, LOWERCASE_CHARACTERS);
		if (uppercase) strcat(charBuf, UPPERCASE_CHARACTERS);
		if (special) strcat (charBuf, SPECIAL_CHARACTERS);

		// reset the buffer size to the buffer's length
		bufSize = strlen(charBuf);

		if (bufSize == 0) {
			error = 1;
			BFErrorPrint("Please provide at least one character option");
		}
	}

	if (!error) {
		// set result
		int i = 0;
		for (; i < size; i++) {
			int r = rand() % bufSize;
			result[i] = charBuf[r];
		}
		result[i] = '\0';
	}
	
	if (err) *err = error;

	return result;
}

