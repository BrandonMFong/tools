/**
 * author: brando
 * date: 10/9/23
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <dirent.h>
#include <string.h>
#include <bflibc/filesystem.h>

#ifdef LINUX
#include <linux/limits.h>
#endif 

#define ARG_FILENAME "-filename"

typedef struct {
	char filename[PATH_MAX];
} SearchOptions;

int Search(const char * inpath, const SearchOptions * opts);
void ParseSearchOptions(int argc, char ** argv, SearchOptions * opts);

void help(const char * toolname) {
	printf("usage: %s <options> <path>\n", toolname);

	printf("\noptions:\n");
	printf("  [ %s ] : searches for files with filename (basename + extension)\n", ARG_FILENAME);

	printf("\nCopyright © 2024 Brando. All rights reserved.\n"); // make this global
}

int main(int argc, char ** argv) {
	int error = 0;
	char path[PATH_MAX];
	bool okayToContinue = true;
	SearchOptions options;

	memset(&options, 0, sizeof(SearchOptions));

	if (argc < 2) {
		okayToContinue = false;
	} else {
		ParseSearchOptions(argc, argv, &options);
		// get path
		if (realpath(argv[argc - 1], path) == 0) { // get abs path
			error = -1;
		}
	}

	if (!error) {
		if (!okayToContinue) {
			error = 1;
			help(argv[0]);
		} else {
			error = Search(path, &options);
		}
	}

	return 0;
}

void ParseSearchOptions(int argc, char ** argv, SearchOptions * opts) {
	if (opts) {
		for (int i = 1; i < (argc - 1); i++) {
			printf("arg: %s\n", argv[i]);
			if (!strcmp(argv[i], ARG_FILENAME)) {
				i++;
				printf("value: %s\n", argv[i]);
				strcpy(opts->filename, argv[i]);
			}
		}
	}
}

int Search(const char * inpath, const SearchOptions * opts) {
	if (!inpath) return -2;
	else if (BFFileSystemPathIsFile(inpath)) {
		printf("file: %s\n", inpath);
	} else {
		printf("dir: %s\n", inpath);
		int error = 0;
		DIR * dir = opendir(inpath);

		if (!dir) return -2;
		else {
			struct dirent * entry = 0;
			while ((entry = readdir(dir)) != NULL) {
				if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
					char path[PATH_MAX];
					snprintf(path, PATH_MAX, "%s/%s", inpath, entry->d_name);
					error = Search(path, opts);
					if (error) break;
				}
			}
		}
		closedir(dir);
		if (error) return error;
	}

	return 0;
}
