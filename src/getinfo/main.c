/**
 * author: brando
 * date: 5/15/24
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctools/tools.h>
#include <limits.h>
#include <bflibc/bflibc.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>

#ifdef LINUX
#include <linux/limits.h>
#endif

#define ARG_FLAG_RECURSIVE 'r'
#define ARG_FLAG_HELP 'h'
#define ARG_BRIEF_DESCRIPTION "--brief-description"

#define STAT_MOD_TYPE_BDEV 'b'
#define STAT_MOD_TYPE_CDEV 'c'
#define STAT_MOD_TYPE_DIR 'd'
#define STAT_MOD_TYPE_FIFO 'p'
#define STAT_MOD_TYPE_SYMLINK 'l'
#define STAT_MOD_TYPE_FILE 'f'
#define STAT_MOD_TYPE_SOCKET 's'
#define STAT_MOD_TYPE_UNKNOWN '?'

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void help(const char * toolname) {
	printf("usage: %s [ -<flags> ] <path>\n", toolname);

	printf("\nflags:\n");
	printf("  [ %c ] : see help text\n", ARG_FLAG_HELP);
	printf("  [ %c ] : recursive\n", ARG_FLAG_RECURSIVE);

	printf("\n");
	printf("entry types:\n");
	printf("  %c - block device\n", STAT_MOD_TYPE_BDEV);
	printf("  %c - char device\n", STAT_MOD_TYPE_CDEV);
	printf("  %c - directory\n", STAT_MOD_TYPE_DIR);
	printf("  %c - fifo pipe\n", STAT_MOD_TYPE_FIFO);
	printf("  %c - symbolic link file\n", STAT_MOD_TYPE_SYMLINK);
	printf("  %c - regular file\n", STAT_MOD_TYPE_FILE);
	printf("  %c - socket\n", STAT_MOD_TYPE_SOCKET);
	printf("  %c - unknown\n", STAT_MOD_TYPE_UNKNOWN);
	printf("\n");
	printf("permissions:\n");
	printf("  <owner><group><other>\n");

	PRINT_COPYRIGHT;
}

void BriefDescription() {
	printf("lists directory\n");
}

typedef struct {
	char path[PATH_MAX];
	int showhelp : 1;
	int recursive : 1;
	bool briefDescription;
} Arguments;

int ArgumentsRead(int argc, char * argv[], Arguments * args) {
	if (argc > 2) {
		printf("error: too many arguments\n");
		return 1;
	} else if (!args) {
		printf("error: params emtpy\n");
		return 1;
	} else if (argc == 1) {
		// no path provided
		// should show current dir
		return 0;
	}

	char arg[32];
	strncpy(arg, argv[1], sizeof(arg));
	const size_t len = strlen(arg);
	if (len == 0) {
		printf("error: argument is emptpy somehow\n");
		return 1;
	} else if (!strcmp(arg, ARG_BRIEF_DESCRIPTION)) {
		args->briefDescription = true;
	} else if (arg[0] != '-') { // if not a flag
		strncpy(args->path, arg, PATH_MAX);
	} else {
		for (int i = 1; i < len; i++) {
			if (arg[i] == ARG_FLAG_RECURSIVE) {
				args->recursive = 0x01;
			} else if (arg[i] == ARG_FLAG_HELP) {
				args->showhelp = 0x01;
			}
		}
	}

	return 0;
}

int ListDir(const Arguments * args);

int main(int argc, char * argv[]) {
	Arguments args;
	memset(&args, 0, sizeof(args));

	// default is current path
	strncpy(args.path, ".", PATH_MAX);

	int error = ArgumentsRead(argc, argv, &args);

	if (!error) {
		if (args.showhelp) {
			help(argv[0]);
		} else if (args.briefDescription) {
			BriefDescription();
		} else {
			ListDir(&args);
		}
	}

	return 0;
}

const char StatGetModeType(struct stat * st) {
	switch (st->st_mode & S_IFMT) {
	case S_IFBLK:	return STAT_MOD_TYPE_BDEV;
	case S_IFCHR:	return STAT_MOD_TYPE_CDEV;
	case S_IFDIR:	return STAT_MOD_TYPE_DIR;
	case S_IFIFO:	return STAT_MOD_TYPE_FIFO;
	case S_IFLNK:	return STAT_MOD_TYPE_SYMLINK;
	case S_IFREG:	return STAT_MOD_TYPE_FILE;
	case S_IFSOCK: 	return STAT_MOD_TYPE_SOCKET;
	default: 		return STAT_MOD_TYPE_UNKNOWN;
	}
}

/**
 * buf : buffer that will hold date
 * bufsize : size of the buf
 */
int StatGetModDate(struct stat * st, char * buf, size_t bufsize) {
	if (!st && !buf) {
		return 1;
	}
	BFDateTime dt;
	BFTime time = st->st_mtime;
	BFTimeGetDateTimeLocal(time, &dt);

	snprintf(buf, bufsize, "%02d/%02d/%02d - %02d:%02d:%02d", dt.month, dt.day, dt.year,
			dt.hour, dt.minute, dt.second);
	return 0;
}

const char * StatGetModeTypeColor(struct stat * st) {
	switch (st->st_mode & S_IFMT) {
	case S_IFBLK:	return ANSI_COLOR_RED; // STAT_MOD_TYPE_BDEV;
	case S_IFCHR:	return ANSI_COLOR_RED; // STAT_MOD_TYPE_CDEV;
	case S_IFDIR:	return ANSI_COLOR_MAGENTA; // STAT_MOD_TYPE_DIR;
	case S_IFIFO:	return ANSI_COLOR_RED; // STAT_MOD_TYPE_FIFO;
	case S_IFLNK:	return ANSI_COLOR_CYAN; // STAT_MOD_TYPE_SYMLINK;
	case S_IFREG:	return ANSI_COLOR_GREEN; // STAT_MOD_TYPE_FILE;
	case S_IFSOCK: 	return ANSI_COLOR_RED; // STAT_MOD_TYPE_SOCKET;
	default: 		return ANSI_COLOR_GREEN; // STAT_MOD_TYPE_UNKNOWN;
	}
}

int PrintPath(const char * path, const Arguments * args) {
	char tmp[PATH_MAX]; 
	char * base = NULL;
	strncpy(tmp, path, PATH_MAX);
	base = basename(tmp);

	// get info
	struct stat st;
	if (stat(path, &st) == -1) {
		printf("error: stat\n");
		return 1;
	}

	// get date
	char dt[64];
	StatGetModDate(&st, dt, sizeof(dt));

	// get size of entry
	// will not do recursion
	size_t size = st.st_size;
	char sizebuf[64];
	int error = BFByteGetString(size, 0, sizebuf);
	if (error)
		return error;

	// get permissions
	const mode_t m = st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

	const char modetype = StatGetModeType(&st);
	const char * color = StatGetModeTypeColor(&st);

	printf("| %-1c-%03o %-21s %10s %s%s%s\n", modetype, m, dt, sizebuf,
			color,
			base,
			ANSI_COLOR_RESET);

	return 0;
}

int ListDir(const Arguments * args) {
	if (!args) {
		printf("error: args is empty\n");
		return 1;
	} else if (!BFFileSystemPathExists(args->path)) {
		printf("error: path '%s' does not exist\n", args->path);
		return 1;
	}

	if (BFFileSystemPathIsFile(args->path)) {
		return PrintPath(args->path, args);
	} else {
		char currpath[PATH_MAX];
		strncpy(currpath, args->path, PATH_MAX);

		struct dirent ** namelist = NULL;
		int n = scandir(currpath, &namelist, NULL, alphasort);
		if (n == -1) {
			printf("error: couldn't scan dir\n");
			return 1;
		}

		for (int i = 0; i < n; i++) {
			if (strcmp(namelist[i]->d_name, ".") && strcmp(namelist[i]->d_name, "..")) {
				char path[PATH_MAX];
				snprintf(path, PATH_MAX, "%s/%s", currpath, namelist[i]->d_name);
				if (args->recursive && (namelist[i]->d_type == DT_DIR)) { // is dir
				} else {
					if (PrintPath(path, args)) {
						printf("error: couldn't print path\n");
						return 1;
					}
				}
			}
			free(namelist[i]);
		}
		free(namelist);
	}

	return 0;
}

