/**
 * author: Brando
 * date: 6/9/2022
 */

#include <bflibc/bflibc.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

char TOOL_ARG[PATH_MAX];
const char * RECURS_ARG = "-r";

void Help() {
	printf("usage: %s [ %s ] <path>\n", TOOL_ARG, RECURS_ARG);

	printf("\nArguments:\n");
	printf("\t%s : Recursively runs through path (if possible) to count items\n", RECURS_ARG);
}

/**
 * Counts the items at path and (if recursive=true) its child directories
 */
unsigned long long CountItemsInPath(const char * path, bool recursive, int * err) {
	unsigned long long result = 0;
	int error = 0;
	DIR * dir = 0;
	struct dirent * derec = 0;
	char s1[PATH_MAX];

	if (BFFileSystemPathIsFile(path)) {
		result++;
	} else if (BFFileSystemPathIsDirectory(path)) {
		if (!(dir = opendir(path))) {
			error = 1;
			BFErrorPrint("Could not read directory: %s", path);
		}

		if (!error) {
			while ((derec = readdir(dir)) && !error) {
				if (strcmp(derec->d_name, ".") && strcmp(derec->d_name, "..")) {
					if (recursive) {
						sprintf(s1, "%s/%s", path, derec->d_name);
						result += CountItemsInPath(s1, recursive, &error);
					} else {
						result++;
					}
				}
			}
		}

		if (dir) {
			if (closedir(dir)) {
				BFErrorPrint("Error finishing reading directory: %s", path);
			}
		}
	} else { 
		BFErrorPrint("Unknown item '%s'", path);
	}

	if (err != 0) {
		*err = error;
	}

	return result;
}

int main(int argc, char * argv[]) {
	int result = 0;
	bool recursive = false;

	strcpy(TOOL_ARG, argv[0]);

	if ((argc != 3) && (argc != 2)) {
		Help();
	} else {
		const char * path = argv[argc - 1];

		if (!path) {
			BFErrorPrint("The path argument is null!");
			result = 1;
		}

		if (!result) {
			recursive = BFArrayStringContainsString(argv, argc, RECURS_ARG);

			unsigned long long count = CountItemsInPath(path, recursive, &result);

			if (!result) {
				printf("%llu\n", count);
			}
		}
	}

	return result;
}
