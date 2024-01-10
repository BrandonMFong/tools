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

#define BUFFER_READ_SIZE 1024
#define SEARCH_DIRECTORY_STACK_SIZE 1024

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
	char word[PATH_MAX];
} SearchOptions;

/**
 * true no options were given
 */
bool SearchOptionsNone(const SearchOptions * opts) {
	if (!opts) return false;
	return !strlen(opts->fullname) &&
		!strlen(opts->ext) &&
		!strlen(opts->name) && 
		!strlen(opts->name) && 
		!strlen(opts->dir) &&
		!strlen(opts->word)
		;
}

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
	printf("  [ %s <string> ] : searches files for an occurrence of word\n", ARG_SEARCH_OPTION_WORD);

	printf("\nCopyright © 2024 Brando. All rights reserved.\n"); // make this global
}

int Search(const char * inpath, const SearchOptions * opts, const SearchFlags flags);
int ParseArguments(int argc, char ** argv, SearchOptions * opts, char * outpath, SearchFlags * flags);

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
		error = Search(path, &options, flags);
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
		} else if (!strcmp(argv[i], ARG_SEARCH_OPTION_WORD)) {
			i++; strcpy(opts->word, argv[i]);
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
 * goes through the options and sees if there are option 
 * combinations that are not allowed
 */
int SearchOptionsAudit(const SearchOptions * opts) {
	if (!opts) return -6;

	if (strlen(opts->fullname) && (strlen(opts->name) || strlen(opts->ext))) {
		printf("syntax: Cannot use %s or %s with %s\n",
				ARG_SEARCH_OPTION_NAME,
				ARG_SEARCH_OPTION_EXTENSION,
				ARG_SEARCH_OPTION_FULLNAME);
		return -6;
	} else if (strlen(opts->dir) && (strlen(opts->fullname) || strlen(opts->name) || strlen(opts->ext))) {
		printf("syntax: Cannot use %s with %s, %s, %s, or %s\n",
				ARG_SEARCH_OPTION_DIR,
				ARG_SEARCH_OPTION_NAME,
				ARG_SEARCH_OPTION_NAME,
				ARG_SEARCH_OPTION_EXTENSION,
				ARG_SEARCH_OPTION_FULLNAME);
		return -6;
	} else if (strlen(opts->word) && (strlen(opts->dir) || strlen(opts->fullname) || strlen(opts->name) || strlen(opts->ext))) {
		printf("syntax: cannot pass %s, %s, %s, %s, or %s when looking for a word\n", 
				ARG_SEARCH_OPTION_DIR,
				ARG_SEARCH_OPTION_NAME,
				ARG_SEARCH_OPTION_NAME,
				ARG_SEARCH_OPTION_EXTENSION,
				ARG_SEARCH_OPTION_FULLNAME);
		return -6;
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

	// audit search options
	if (!error) {
		error = SearchOptionsAudit(opts);
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
 * reads file from inpath for any occurrence of the `word` on every line
 *
 * param lines: caller owns memory and must free()
 *
 * return true if word was found. `lines` should contain an output string shows 
 * location of the word. You can print out the entire var `lines`
 */
bool ExamineFileForWord(
		const char * inpath,
		const char * word,
		const SearchFlags flags,
		char ** lines
) {
	if (!inpath || !word || !lines) return false;

	// open file
	FILE * file = fopen(inpath, "r");
	if (!file) return false;

	// read file
	char buf[BUFFER_READ_SIZE];
	int lineindex = 1;
	bool foundword = false; // true if at least one occurrence is found
	while ((fgets(buf, BUFFER_READ_SIZE, file)) != NULL) { // read line
		// print out first occurrence of word
		if (strstr(buf, word)) {
			foundword = true;

			// we will return the lines if we are in verbose mode
			if (flags & FLAG_BIT_VERBOSE) {
				// figure out size of string we will be appending
				//
				// `s` will hold size of string, not including the null
				// char
				size_t s = snprintf(0, 0, "%d: %s", lineindex, buf);
				if (s) {
					// craft the appending string
					char * tmp = (char *) malloc(sizeof(char) * (s + 1));
					snprintf(tmp, s+1, "%d: %s", lineindex, buf);

					// resize the var
					size_t lsize = *lines ? strlen(*lines) : 0;
					*lines = (char *) realloc(*lines, sizeof(char) * (lsize + strlen(tmp) + 1));
					(*lines)[0] = '\0';

					// craft the string
					strcat(*lines, tmp);

					free(tmp);
				}
			}
		}
		lineindex++;
	}

	// close
	fclose(file);

	return foundword;
}

/**
 * For files, we look at the inpath based on options provider
 */
void ExamineFile(const char * inpath, const SearchOptions * opts, const SearchFlags flags) {
	bool print = false;
	char * lines = NULL;

	if (!opts) return;

	if (SearchOptionsNone(opts)) { // if no opts, show inpath
		print = true;
	} else if (strlen(opts->fullname)) { // if fullname was passed
		if (SearchOptionsMatchFullname(inpath, opts)) {
			print = true;
		}
	} else if (strlen(opts->ext) && strlen(opts->name)) { // if extension and name
		if (SearchOptionsMatchExtension(inpath, opts) &&
			SearchOptionsMatchName(inpath, opts)) {
			print = true;
		}
	} else if (strlen(opts->ext)) { // if extension
		if (SearchOptionsMatchExtension(inpath, opts)) {
			print = true;
		}
	} else if (strlen(opts->name)) { // if name
		if (SearchOptionsMatchName(inpath, opts)) {
			print = true;
		}
	} else if (strlen(opts->word)) { // find word
		print = ExamineFileForWord(inpath, opts->word, flags, &lines);
	}

	if (print) {
		if (flags & (FLAG_BIT_VERBOSE)) {
			printf("f: %s\n", inpath);

			// print out `lines` if available			
			if (lines) {
				printf("%s", lines);
			}
		} else {
			printf("%s\n", inpath);
		}
	}

	free(lines);
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
			printf("d: %s\n", inpath);
		} else {
			printf("%s\n", inpath);
		}
	}
}

int SearchDirectoryRecursively(const char * dirpath, const SearchOptions * opts, const SearchFlags flags);

/**
 * Searches inpath according to search options
 *
 * param inpath: path to search
 * param opts: search options
 * param flags: search flags
 */
int Search(const char * inpath, const SearchOptions * opts, const SearchFlags flags) {
	int error = 0;
	if (!inpath || !opts) error = -2; // null inpath
	else if (BFFileSystemPathIsFile(inpath)) { // if file
		ExamineFile(inpath, opts, flags);
	} else { // if dir
		ExamineDirectory(inpath, opts, flags);
		if (flags & FLAG_BIT_RECURSIVE) {
			SearchDirectoryRecursively(inpath, opts, flags);
		}
	}

	return error;
}

/**
 * Custom stack that helps manage stackoverflows
 */
typedef struct {
	char dirs[SEARCH_DIRECTORY_STACK_SIZE][PATH_MAX];
	int size;
} SearchDirectoryStack;

/// Stack Push
int SearchDirectoryStackPush(SearchDirectoryStack * stack, const char * dir) {
	if (stack && dir) {
		if (stack->size < SEARCH_DIRECTORY_STACK_SIZE) {
			strcpy(stack->dirs[stack->size++], dir);
		} else {
			printf("Search Directory Stack Overflow\n");
			return -14;
		}
	}
	return 0;
}

/// Stack Pop
void SearchDirectoryStackPop(SearchDirectoryStack * stack, char * dir) {
	if (stack && dir && (stack->size > 0)) {
		strcpy(dir, stack->dirs[(stack->size--) - 1]);
	}
}

/**
 * Recursively looks into dirpath's tree
 *
 * This function does not do recursion but rather uses a custom stack
 *
 * param dirpath: directory path
 */
int SearchDirectoryRecursively(const char * dirpath, const SearchOptions * opts, const SearchFlags flags) {
	int error = 0;
	SearchDirectoryStack dirstack;
	if (!dirpath || !opts) return -13;

	// init stack
	memset(&dirstack, 0, sizeof(SearchDirectoryStack));
	error = SearchDirectoryStackPush(&dirstack, dirpath);

	// run through entire directory tree
	while (!error && dirstack.size) {
		// get top of the stack
		char currpath[PATH_MAX];
		SearchDirectoryStackPop(&dirstack, currpath);

		// First examine directory
		ExamineDirectory(currpath, opts, flags);

		DIR * dir = opendir(currpath);
		if (!dir) {
			if (flags & FLAG_BIT_VERBOSE) {
				printf("could not look into: %s\n", currpath);
			}
		} else {
			struct dirent * entry = 0;
			while (!error && ((entry = readdir(dir)) != NULL)) {
				if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
					char path[PATH_MAX];
					snprintf(path, PATH_MAX, "%s/%s", currpath, entry->d_name);

					// if path is a dir then we will push it to the stack
					// and will work on it later
					//
					// if file, then we can call ExamineFile
					if (entry->d_type == DT_DIR) { // is dir
						error = SearchDirectoryStackPush(&dirstack, path);
					} else { // is file
						ExamineFile(path, opts, flags);
					}
				}
			}

			closedir(dir);
		}

	}

	return 0;
}

