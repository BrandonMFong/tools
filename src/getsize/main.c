/**
 * author: Brando 
 * date: 5/1/2022
 */

#include <bflibc/bflibc.h>
#include <ctools/tools.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <libgen.h>
#include <limits.h>
#include <ctools/tools.h>

#ifdef LINUX
#include <linux/limits.h>
#endif

/**
 * Allows user to show each path we find
 */
const char * VERBOSE_ARG = "-v";

/**
 * prints out brief description of what we do
 */
const char * BRIEF_DESCRIPTION = "--brief-description";

void Help(const char * toolname) {
	printf("usage: %s [ %s ] <path>\n", toolname, VERBOSE_ARG);

	printf("\nArguments:\n");
	printf("\t%s : Verbose mode\n", VERBOSE_ARG);
	printf("\t<path> : Can be absolute or relative.  Cannot be a symbolic link\n");
}

void BriefDescription();
int ParseArguments(int argc, char ** argv, char * path, bool * brieflyDescribe, unsigned char * options);
int GetSize(const char * path, unsigned char options);

int TOOL_MAIN(int argc, char * argv[]) {
	int result = 0;
	unsigned long long size = 0;
	char path[PATH_MAX];
	unsigned char options = 0;
	bool brieflyDescribe = false;

	if (!result) {
		result = ParseArguments(argc, argv, path, &brieflyDescribe, &options);
	}

	if (result) {
		Help(argv[0]);
	} else {
		if (brieflyDescribe) {
			BriefDescription();
		} else {
			result = GetSize(path, options);
		}
	}


	return result;
}

void BriefDescription() {
	printf("returns size of file\n");
}

int ParseArguments(int argc, char ** argv, char * path, bool * brieflyDescribe, unsigned char * options) {
	if (!argv || !path || !brieflyDescribe || !options) return -2;
	else if (argc < 2) return -2;
	for (int i = 1; i < argc; i++) {
		if (i == (argc - 1)) { // path
			strcpy(path, argv[i]);
		} else if (!strcmp(argv[i], BRIEF_DESCRIPTION)) {
			*brieflyDescribe = true;
		} else if (!strcmp(argv[i], VERBOSE_ARG)) {
			*options |= kCalculateSizeOptionsVerbose;
		} else {
			printf("unknown arg: %s\n", argv[i]);
			return -2;
		}
	}
	return 0;
}

int PathGetSizeInBytes(const char * path, unsigned char options, int * size) {
	if (!path || !size) return -4;

	// Check what type of path this is
	// and if depending of the type, we 
	// will calculate the total size of it
	int error = 0;
	if (BFFileSystemPathIsSymbolicLink(path)) {
		printf("Path '%s' is a symbolic link.  We cannot calculate size for sym links\n", path);
		error = -4;
	} else if (BFFileSystemPathIsFile(path)) {
		*size = BFFileSystemFileGetSizeUsed(path, options, &error);
	} else if (BFFileSystemPathIsDirectory(path)) {
		*size = BFFileSystemDirectoryGetSizeUsed(path, options, &error);
	} else {
		printf("Unknown file type for '%s'\n", path);
		error = -4;
	}

	return error;	
}

/**
 * Prints the byteSize value into a format that represents
 * a comprehensible byte format
 */
int PrintSize(unsigned long long byteSize) {
	char unit[10];
	
	if (BFByteGetString(byteSize, unit)) {
		BFErrorPrint("Error with getting byte representation");
	} else {
		printf("%s\n", unit);
	}

	return 0;
}

int GetSize(const char * path, unsigned char options) {
	if (!path) return -3;
	
	// Make sure the path the user provided exists
	if (!BFFileSystemPathExists(path)) {
		printf("Path '%s' does not exist!\n", path);
		return -3;
	}

	// get the size in bytes
	int size = 0;
	int error = PathGetSizeInBytes(path, options, &size);

	// Display size
	if (!error) {
		error = PrintSize(size);
	}
	
	return error;
}

#ifdef TESTING

#include <bflibc/bftests.h>

int test_GettingSizeOfFile(void) {
	UNIT_TEST_START;
	int result = 0;
	UNIT_TEST_END(!result, result);
	return result;
}

int test_ParseArguments(void) {
	UNIT_TEST_START;
	int result = 0;
	int err = 0;
	bool brieflyDescribe = false;
	unsigned char options = 0;
	char path[PATH_MAX];

	// no args
	char *args[] = {"getsize"};
	err = ParseArguments(sizeof(args) / sizeof(args[0]), args, path, &brieflyDescribe, &options);
	if (err == 0) result = -1;

	// just path
	if (!result) {
		path[0] = '\0';
		char *args[] = {"getsize", "/tmp/path"};
		err = ParseArguments(sizeof(args) / sizeof(args[0]), args, path, &brieflyDescribe, &options);
		if (err != 0) result = -2;
		else if (strlen(path) == 0) result = -3;
	}

	UNIT_TEST_END(!result, result);
	return result;
}

int TOOL_TEST(int argc, char ** argv) {
	int p = 0, f = 0;
	printf("TESTING: %s\n", argv[0]);

	LAUNCH_TEST(test_GettingSizeOfFile, p, f);
	LAUNCH_TEST(test_ParseArguments, p, f);

	PRINT_GRADE(p, f);

	return 0;
}

#endif // TESTING

