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

#define ARG_FILENAME "-fullname"
#define ARG_EXTENSION "-ext"

typedef struct {
	char filename[PATH_MAX];
	char ext[PATH_MAX];
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
			if (!strcmp(argv[i], ARG_FILENAME)) {
				i++; strcpy(opts->filename, argv[i]);
			} else if (!strcmp(argv[i], ARG_EXTENSION)) {
				i++; strcpy(opts->ext, argv[i]);
			}
		}
	}
}

/**
 * true no options were found by ParseSearchOptions
 */
bool SearchOptionsNone(const SearchOptions * opts) {
	if (!opts) return false;
	return !strlen(opts->filename) && !strlen(opts->ext);
}

bool SearchOptionsMatchFilename(const char * inpath, const SearchOptions * opts) {
	if (opts && strlen(opts->filename)) {
		char fullname[PATH_MAX];
		int error = BFFileSystemPathGetFullname(inpath, fullname);
		if (error) {
			printf("BFFileSystemPathGetFullname - %d\n", error);
		} else {
			return !strcmp(opts->filename, fullname);
		}
	}

	return false;
}

bool SearchOptionsMatchExtension(const char * inpath, const SearchOptions * opts) {
	if (opts && strlen(opts->ext)) {
		char ext[PATH_MAX];
		int error = BFFileSystemPathGetExtension(inpath, ext);
		if (error) {
			printf("BFFileSystemPathGetExt - %d\n", error);
		} else {
			return !strcmp(opts->ext, ext);
		}
	}

	return false;
}


int Search(const char * inpath, const SearchOptions * opts) {
	if (!inpath) return -2;
	else if (BFFileSystemPathIsFile(inpath)) {
		if (SearchOptionsNone(opts)) { // if no opts, show
			printf("%s\n", inpath);
		} else if (
				SearchOptionsMatchFilename(inpath, opts) ||
				SearchOptionsMatchExtension(inpath, opts)) {
			printf("%s\n", inpath);
		}
	} else {
		int error = 0;

		if (SearchOptionsNone(opts)) {
			printf("%s\n", inpath);
		}

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

