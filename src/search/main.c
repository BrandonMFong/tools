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

#define ARG_SEARCH_OPTION_FULLNAME "-fullname"
#define ARG_SEARCH_OPTION_EXTENSION "-ext"
#define ARG_SEARCH_OPTION_NAME "-name"
#define ARG_SEARCH_OPTION_DIR "-dir"
#define ARG_SEARCH_OPTION_WORD "-word"

typedef struct {
	char fullname[PATH_MAX];
	char ext[PATH_MAX];
	char name[PATH_MAX];
	char dir[PATH_MAX];
} SearchOptions;

int Search(const char * inpath, const SearchOptions * opts);
int ParseArguments(int argc, char ** argv, SearchOptions * opts, char * outpath);

void help(const char * toolname) {
	printf("usage: %s <options> <path>\n", toolname);

	printf("\noptions:\n");
	printf("  [ %s <string> ] : searches for files with fullname (basename + extension)\n", ARG_SEARCH_OPTION_FULLNAME);
	printf("  [ %s <string> ] : searches for files with extension\n", ARG_SEARCH_OPTION_EXTENSION);
	printf("  [ %s <string> ] : searches for files with basename\n", ARG_SEARCH_OPTION_NAME);
	printf("  [ %s <string> ] : searches for directory with the same name\n", ARG_SEARCH_OPTION_DIR);
	printf("  [ %s <string> ] : searches files that contain exact word\n", ARG_SEARCH_OPTION_WORD);

	printf("\nCopyright © 2024 Brando. All rights reserved.\n"); // make this global
}

int main(int argc, char ** argv) {
	int error = 0;
	char path[PATH_MAX];
	SearchOptions options;

	memset(&options, 0, sizeof(SearchOptions));
	memset(&path[0], 0, sizeof(PATH_MAX));

	if (argc < 2) {
		error = 1;
	} else {
		error = ParseArguments(argc, argv, &options, path);
	}

	if (!error) {
		error = Search(path, &options);
	}
	
	if (error) {
		help(argv[0]);
	}

	return error;
}

/**
 * loads SearchOptions with what we find from cmd args
 */
int ParseArguments(int argc, char ** argv, SearchOptions * opts, char * outpath) {
	int error = 0;
	if (!opts || !argv || !outpath) {
		error = -3;
	} else {
		// get search options
		for (int i = 1; i < (argc - 1); i++) {
			if (!strcmp(argv[i], ARG_SEARCH_OPTION_FULLNAME)) {
				i++; strcpy(opts->fullname, argv[i]);
			} else if (!strcmp(argv[i], ARG_SEARCH_OPTION_EXTENSION)) {
				i++; strcpy(opts->ext, argv[i]);
			} else if (!strcmp(argv[i], ARG_SEARCH_OPTION_NAME)) {
				i++; strcpy(opts->name, argv[i]);
			} else if (!strcmp(argv[i], ARG_SEARCH_OPTION_DIR)) {
				i++; strcpy(opts->dir, argv[i]);
			} else {
				printf("unknown option: %s\n", argv[i]);
				error = -3;
				break;
			}
		}

		// get path
		if (realpath(argv[argc - 1], outpath) == 0) { // get abs path
			error = -3;
		}
	}

	return error;
}

/**
 * true no options were given
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

/// fullname
bool SearchOptionsMatchFullname(const char * inpath, const SearchOptions * opts) {
	if (!opts) return false;
	return _SearchOptionsMatchCommon(inpath, opts->fullname, BFFileSystemPathGetFullname);
}

// extension
bool SearchOptionsMatchExtension(const char * inpath, const SearchOptions * opts) {
	if (!opts) return false;
	return _SearchOptionsMatchCommon(inpath, opts->ext, BFFileSystemPathGetExtension);
}

/// directory name
bool SearchOptionsMatchDir(const char * inpath, const SearchOptions * opts) {
	if (!opts) return false;
	return _SearchOptionsMatchCommon(inpath, opts->dir, BFFileSystemPathGetName);
}

/// name (ie basename without extension)
bool SearchOptionsMatchName(const char * inpath, const SearchOptions * opts) {
	if (!opts) return false;
	return _SearchOptionsMatchCommon(inpath, opts->name, BFFileSystemPathGetName);
}

/**
 * For files, we look at the inpath based on options provider
 */
void ExamineFile(const char * inpath, const SearchOptions * opts) {
	if (SearchOptionsNone(opts)) { // if no opts, show
		printf("%s\n", inpath);
	} else if (
			SearchOptionsMatchFullname(inpath, opts) ||
			SearchOptionsMatchExtension(inpath, opts) ||
			SearchOptionsMatchName(inpath, opts)) {
		printf("%s\n", inpath);
	}
}

/**
 * For directory, we look at the inpath based on options provider
 */
void ExamineDirectory(const char * inpath, const SearchOptions * opts) {
	if (SearchOptionsNone(opts)) { // if no opts, show
		printf("%s\n", inpath);
	} else if (SearchOptionsMatchDir(inpath, opts)) {
		printf("%s\n", inpath);
	}
}

int Search(const char * inpath, const SearchOptions * opts) {
	int error = 0;
	if (!inpath) error = -2; // null inpath
	else if (BFFileSystemPathIsFile(inpath)) { // if file
		ExamineFile(inpath, opts);
	} else { // if dir
		ExamineDirectory(inpath, opts);

		// recursively call Search
		
		DIR * dir = opendir(inpath);

		if (!dir) error = -2;
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
	}

	return error;
}

