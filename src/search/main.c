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

#define ARG_UNKNOWN(arg) ( \
		strcmp(arg, ARG_SEARCH_OPTION_FULLNAME) && \
		strcmp(arg, ARG_SEARCH_OPTION_EXTENSION) && \
		strcmp(arg, ARG_SEARCH_OPTION_NAME) && \
		strcmp(arg, ARG_SEARCH_OPTION_DIR) && \
		strcmp(arg, ARG_SEARCH_OPTION_WORD) \
		)

#define ARG_FLAG_VERBOSE 'v'
#define FLAG_BIT_VERBOSE (0x01 << 0)
#define ARG_FLAG_RECURSIVE 'r'
#define FLAG_BIT_RECURSIVE (0x01 << 1)

/**
 * |0|0|0|0|0|0|recursive|verbose|
 */
typedef char SearchFlags;

typedef struct {
	char fullname[PATH_MAX];
	char ext[PATH_MAX];
	char name[PATH_MAX];
	char dir[PATH_MAX];
} SearchOptions;

int Search(const char * inpath, const SearchOptions * opts, const SearchFlags flags, int lvl);
int ParseArguments(int argc, char ** argv, SearchOptions * opts, char * outpath, SearchFlags * flags);

void help(const char * toolname) {
	printf("usage: %s [ -<flags> ] [ <options> ] <path>\n", toolname);

	printf("\nflags:\n");
	printf("  [ %c ] : verbose\n", ARG_FLAG_VERBOSE);
	printf("  [ %c ] : recursive\n", ARG_FLAG_RECURSIVE);

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
	SearchFlags flags = 0;

	memset(&options, 0, sizeof(SearchOptions));
	memset(&path[0], 0, sizeof(PATH_MAX));

	if (argc < 2) {
		error = 1;
	} else {
		error = ParseArguments(argc, argv, &options, path, &flags);
	}

	if (!error) {
		error = Search(path, &options, flags, 0);
	}
	
	if (error) {
		help(argv[0]);
	}

	return error;
}

int SearchOptionsLoadFromArguments(
		int argc,
		char ** argv,
		int startindex,
		SearchOptions * opts
) {
	if (!argv || !opts) return -4;
	for (int i = startindex; i < (argc - 1); i++) {
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
			return -4;
		}
	}

	return 0;
}

/**
 * param arg: 	we can safely assume that this is not one of the
 * 				search options. so this could be an unknown argument
 */
int SearchFlagsLoadFromArgument(const char * arg, SearchFlags * flags) {
	if (!arg || !flags) return -5;
	
	char buf[512];
	strcpy(buf, arg);
	const size_t len = strlen(buf);
	
	// valid flag syntax
	if ((buf[0] == '-') && (buf[1] != '-')) {
		for (int i = 1; i < len; i++) {
			if (buf[i] == ARG_FLAG_VERBOSE) {
				*flags |= (FLAG_BIT_VERBOSE);
			} else if (buf[i] == ARG_FLAG_RECURSIVE) {
				*flags |= (FLAG_BIT_RECURSIVE);
			}
		}
	}

	return 0;
}

/**
 * loads SearchOptions with what we find from cmd args
 */
int ParseArguments(
		int argc, 
		char ** argv, 
		SearchOptions * opts, 
		char * outpath, 
		SearchFlags * flags
) {
	int error = 0;
	int i = 1;

	if (!opts || !argv || !outpath) {
		error = -3;
	
	// if unknown, see if these are flags
	} else if (ARG_UNKNOWN(argv[i])) {
		error = SearchFlagsLoadFromArgument(argv[i], flags);
		i++;
	}

	// get search options
	if (!error) {
		error = SearchOptionsLoadFromArguments(argc, argv, i, opts);
	}

	// get path
	if (!error) {
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
		int (* callback) (const char *, char *)
) {
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
void ExamineFile(const char * inpath, const SearchOptions * opts, const SearchFlags flags) {
	bool print = false;
	if (SearchOptionsNone(opts)) { // if no opts, show
		print = true;
	} else if (
			SearchOptionsMatchFullname(inpath, opts) ||
			SearchOptionsMatchExtension(inpath, opts) ||
			SearchOptionsMatchName(inpath, opts)) {
		print = true;
	}

	if (print) {
		if (flags & (FLAG_BIT_VERBOSE)) {
			printf("file: %s\n", inpath);
		} else {
			printf("%s\n", inpath);
		}
	}
}

/**
 * For directory, we look at the inpath based on options provider
 */
void ExamineDirectory(const char * inpath, const SearchOptions * opts, const SearchFlags flags) {
	bool print = false;
	if (SearchOptionsNone(opts)) { // if no opts, show
		printf("%s\n", inpath);
	} else if (SearchOptionsMatchDir(inpath, opts)) {
		printf("%s\n", inpath);
	}

	if (print) {
		if (flags & (FLAG_BIT_VERBOSE)) {
			printf("file: %s\n", inpath);
		} else {
			printf("%s\n", inpath);
		}
	}
}

/**
 * param inpath: path to search
 * param opts: search options
 * param flags: search flags
 * param lvl: recursion level
 */
int Search(const char * inpath, const SearchOptions * opts, const SearchFlags flags, int lvl) {
	int error = 0;
	if (!inpath) error = -2; // null inpath
	else if (BFFileSystemPathIsFile(inpath)) { // if file
		ExamineFile(inpath, opts, flags);
	} else { // if dir
		ExamineDirectory(inpath, opts, flags);

		// recursively call Search
		if ((flags & FLAG_BIT_RECURSIVE) || (lvl < 1)) {
			lvl++;
			DIR * dir = opendir(inpath);

			if (!dir) error = -2;
			else {
				struct dirent * entry = 0;
				while ((entry = readdir(dir)) != NULL) {
					if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
						char path[PATH_MAX];
						snprintf(path, PATH_MAX, "%s/%s", inpath, entry->d_name);
				
						error = Search(path, opts, flags, lvl);
						if (error) break;
					}
				}
			}

			closedir(dir);
		}
	}

	return error;
}

