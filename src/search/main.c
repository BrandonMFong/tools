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

#define ARG_FULLNAME "-fullname"
#define ARG_EXTENSION "-ext"
#define ARG_NAME "-name"
#define ARG_DIR "-dir"
#define ARG_WORD "-word"

typedef struct {
	char fullname[PATH_MAX];
	char ext[PATH_MAX];
	char name[PATH_MAX];
	char dir[PATH_MAX];
} SearchOptions;

int Search(const char * inpath, const SearchOptions * opts);
void ParseSearchOptions(int argc, char ** argv, SearchOptions * opts);

void help(const char * toolname) {
	printf("usage: %s <options> <path>\n", toolname);

	printf("\noptions:\n");
	printf("  [ %s <string> ] : searches for files with fullname (basename + extension)\n", ARG_FULLNAME);
	printf("  [ %s <string> ] : searches for files with extension\n", ARG_EXTENSION);
	printf("  [ %s <string> ] : searches for files with basename\n", ARG_NAME);
	printf("  [ %s <string> ] : searches for directory with the same name\n", ARG_DIR);
	printf("  [ %s <string> ] : searches files that contain exact word\n", ARG_WORD);

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

	return error;
}

/**
 * loads SearchOptions with what we find from cmd args
 */
void ParseSearchOptions(int argc, char ** argv, SearchOptions * opts) {
	if (opts) {
		for (int i = 1; i < (argc - 1); i++) {
			if (!strcmp(argv[i], ARG_FULLNAME)) {
				i++; strcpy(opts->fullname, argv[i]);
			} else if (!strcmp(argv[i], ARG_EXTENSION)) {
				i++; strcpy(opts->ext, argv[i]);
			} else if (!strcmp(argv[i], ARG_NAME)) {
				i++; strcpy(opts->name, argv[i]);
			} else if (!strcmp(argv[i], ARG_DIR)) {
				i++; strcpy(opts->dir, argv[i]);
			}
		}
	}
}

/**
 * true no options were found by ParseSearchOptions
 */
bool SearchOptionsNone(const SearchOptions * opts) {
	if (!opts) return false;
	return !strlen(opts->fullname) &&
		!strlen(opts->ext) &&
		!strlen(opts->name) && 
		!strlen(opts->name) && 
		!strlen(opts->dir);
}

/**
 * common logic that is used when matching base with the result from callback
 *
 * param callback: BFFileSystemPath Getter. This is following a standard spec that should persist in the bflib 
 */
bool _SearchOptionsMatchCommon(
		const char * inpath, 
		const char * base, 
		int (* callback) (const char *, char *)) {
	if (strlen(base)) {
		char t[PATH_MAX];
		int error = callback(inpath, t);
		if (error) {
			printf("error - %d\n", error);
		} else {
			return !strcmp(base, t);
		}
	}

	return false;
}

bool SearchOptionsMatchFullname(const char * inpath, const SearchOptions * opts) {
	if (!opts) return false;
	return _SearchOptionsMatchCommon(inpath, opts->fullname, BFFileSystemPathGetFullname);
}

bool SearchOptionsMatchExtension(const char * inpath, const SearchOptions * opts) {
	if (!opts) return false;
	return _SearchOptionsMatchCommon(inpath, opts->ext, BFFileSystemPathGetExtension);
}

bool SearchOptionsMatchDir(const char * inpath, const SearchOptions * opts) {
	if (!opts) return false;
	return _SearchOptionsMatchCommon(inpath, opts->dir, BFFileSystemPathGetName);
}

bool SearchOptionsMatchName(const char * inpath, const SearchOptions * opts) {
	if (!opts) return false;
	return _SearchOptionsMatchCommon(inpath, opts->name, BFFileSystemPathGetName);
}

int Search(const char * inpath, const SearchOptions * opts) {
	if (!inpath) return -2; // null inpath
	else if (BFFileSystemPathIsFile(inpath)) { // if file
		if (SearchOptionsNone(opts)) { // if no opts, show
			printf("%s\n", inpath);
		} else if (
				SearchOptionsMatchFullname(inpath, opts) ||
				SearchOptionsMatchExtension(inpath, opts) ||
				SearchOptionsMatchName(inpath, opts)) {
			printf("%s\n", inpath);
		}
	} else { // if dir
		int error = 0;

		if (SearchOptionsNone(opts)) { // if no opts, show
			printf("%s\n", inpath);
		} else if (SearchOptionsMatchDir(inpath, opts)) {
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

