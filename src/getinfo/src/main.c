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
#include <errno.h>
#include <pwd.h>

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

#define STAT_MOD_TYPE_DESCRIPTION_BDEV "Block Device"
#define STAT_MOD_TYPE_DESCRIPTION_CDEV "Character Device"
#define STAT_MOD_TYPE_DESCRIPTION_DIR "Directory"
#define STAT_MOD_TYPE_DESCRIPTION_FIFO "Fifo Pipe File"
#define STAT_MOD_TYPE_DESCRIPTION_SYMLINK "Symlink File"
#define STAT_MOD_TYPE_DESCRIPTION_FILE "Regular File"
#define STAT_MOD_TYPE_DESCRIPTION_SOCKET "Socket"
#define STAT_MOD_TYPE_DESCRIPTION_UNKNOWN "Unknown"

// https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

typedef struct PathQuery {
	char p[PATH_MAX]; // path (relative/absolute)
	
	/**
	 * parent path, should be a directory
	 *
	 * this is a weak reference pointer. we do not
	 * own memory and it isn't dynamically allocated.
	 *
	 * parent pathquery memory will always exist when there
	 * is a child.
	 */
	const struct PathQuery * parent;

	/**
	 * level of path
	 *
	 * the input path the user provides will always start at level
	 * 0. Every child made after will be incremented
	 */
	unsigned char lvl;
} PathQuery;

/**
 * there are two different arrays but the accessor functions
 * should abstract the data retrieval
 */
typedef struct {
	/// for files
	char ** arrayfile;
	size_t sizefile;

	/// for directories
	char ** arraydir;
	size_t sizedir;
} PathList;

typedef struct {
	/**
	 * array of input paths provided by user
	 */
	PathList paths;
	unsigned char showhelp : 1;
	unsigned char recursive : 1;
	unsigned char briefDescription : 1;
} Arguments;

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

int ArgumentsRead(int argc, char * argv[], Arguments * args);
int PathListRelease(PathList * paths);
int GetInfo(const Arguments * args);

int TOOL_MAIN(int argc, char * argv[]) {
	Arguments args;
	memset(&args, 0, sizeof(args));

	int error = ArgumentsRead(argc, argv, &args);

	if (!error) {
		if (args.showhelp) {
			help(argv[0]);
		} else if (args.briefDescription) {
			BriefDescription();
		} else {
			GetInfo(&args);
		}
	}

	if (PathListRelease(&args.paths)) {
		printf("error: couldn't release paths\n");
		return 1;
	}

	return 0;
}

/**
 *
 * removes "./" from the start of the string
 * removes by shifting the characters to the right
 */
int RemoveLeadingPeriodAndForwardSlashes(char * buf) {
	if (!buf)
		return 1;

	int s = 0;
	if (buf[0] == '.' && buf[1] == '/' ) {
		s = 2;
	}

	if (s) {
		memmove(buf, buf+s, strlen(buf + s) + 1);
	}

	return 0;
}

int RemoveTrailingForwardSlashes(char * buf) {
	if (!buf)
		return 1;

	for (int i = strlen(buf) - 1; i > 0; i--) {
		if (buf[i] == '/') {
			buf[i] = '\0';
		} else {
			break;
		}
	}
	
	return 0;
}

int PathQueryCreate(PathQuery * p, const char * path) {
	if (!p || !path) return 1;

	memset(p, 0, sizeof(PathQuery));
	strncpy(p->p, path, PATH_MAX);
	RemoveTrailingForwardSlashes(p->p);
	
	return 0;
}

/**
 * creates child path query
 *
 * @param leaf the leaf component. PathQueryGetPath() will return full path as it will
 * consider its parent's path
 */
int PathQueryCreateChild(
	const PathQuery * p,
	PathQuery * c,
	const char * leaf
) {
	if (!p || !leaf || !c) return 1;

	memset(c, 0, sizeof(PathQuery));
	strncpy(c->p, leaf, PATH_MAX);
	c->parent = p;
	c->lvl = p->lvl + 1;
	
	RemoveTrailingForwardSlashes(c->p);
	RemoveLeadingPeriodAndForwardSlashes(c->p);

	return 0;
}

int PathQueryGetPath(const PathQuery * p, char * buf) {
	if (!p || !buf) return 1;

	char path[PATH_MAX];
	memset(path, 0, PATH_MAX);

	const PathQuery * q = p;
	do {
		char t[PATH_MAX];

		// prepend current path to our main buf
		snprintf(t, PATH_MAX, "%s/%s", q->p, path);
		strncpy(path, t, PATH_MAX);
	} while ((q = q->parent) != NULL);

	RemoveTrailingForwardSlashes(path);

	strncpy(buf, path, PATH_MAX);

	return 0;
}

int PathQueryRelease(PathQuery * p) {
	return 0;
}

unsigned char PathQueryGetLevel(const PathQuery * p) {
	if (!p) return 1;
	return p->lvl;
}

bool PathQueryIsFile(const PathQuery * p) {
	if (!p) return false;
	return BFFileSystemPathIsFile(p->p);
}

size_t PathListGetSize(const PathList * paths) {
	if (!paths) return 0;
	return paths->sizefile + paths->sizedir;
}

int PathListGetPathAtIndex(const PathList * paths, size_t index, char * buf) {
	if (!paths || !buf) return 1;
	else if (index >= PathListGetSize(paths)) return 1;

	if (index < (paths->sizefile))
		strncpy(buf, paths->arrayfile[index], strlen(paths->arrayfile[index]) + 1);
	else
		strncpy(buf,
				paths->arraydir[index - paths->sizefile],
				strlen(paths->arraydir[index - paths->sizefile]) + 1);

	return 0;
}

int ArgumentsReadFlagsFromArg(const char * arg, Arguments * args) {
	if (!arg && !args) {
		printf("error: param error\n");
		return 1;
	}

	size_t s = strlen(arg);
	for (int i = 0; i < s; i++) {
		if (arg[i] == ARG_FLAG_RECURSIVE) {
			args->recursive = true;
		} else if (arg[i] == ARG_FLAG_HELP) {
			args->showhelp = true;
		}
	}

	return 0;
}

/**
 * selection sort
 *
 * sorts in ascending order
 */
int ArraySort(char ** array, size_t size) {
	if (!array) return 1;
	
	for (int i = 0; i < size; i++) {
		// find min
		int min = i;
		for (int j = i + 1; j < size; j++) {
			if (strcmp(array[j], array[min]) < 0) {
				min = j;
			}
		}

		if (min != i) {
			char * tmp = array[min];
			array[min] = array[i];
			array[i] = tmp;
		}
	}

	return 0;
}

int PathListRelease(PathList * paths) {
	if (!paths) return 1;

	for (int i = 0; i < paths->sizefile; i++) {
		char * tmp = paths->arrayfile[i];
		BFFree(tmp);
	}

	for (int i = 0; i < paths->sizedir; i++) {
		char * tmp = paths->arraydir[i];
		BFFree(tmp);
	}

	BFFree(paths->arrayfile);
	BFFree(paths->arraydir);

	return 0;
}

int AddPathToArray(void ** arrayptr, size_t * size, const char * path) {
	if (!arrayptr || !size || !path) return 1;

	char ** array = (char **) *arrayptr;

	array = (char **) realloc(array, sizeof(char **) * ++(*size));
	if (array == NULL) {
		printf("error: couldn't allocate more space for path array (size %ld)\n", *size);
		return 1;
	}

	char * tmp = BFStringCopyString(path);
	if (tmp == NULL) {
		printf("error: couldn't allocate memory for string %s\n", path);
		return 1;
	}

	array[*size - 1] = tmp;

	*arrayptr = array;

	return 0;
}

int PathListAddPath(PathList * paths, const char * path) {
	if (!paths || !path) {
		printf("error: param error\n");
		return 1;
	}

	void ** array = NULL;
	size_t * size = 0;
	if (BFFileSystemPathIsFile(path)) {
		array = (void **) &paths->arrayfile;
		size = &paths->sizefile;
	} else {
		array = (void **) &paths->arraydir;
		size = &paths->sizedir;
	}

	return AddPathToArray(array, size, path);
}

int PathListSort(PathList * paths) {
	if (!paths) return 1;

	if (paths->arrayfile && ArraySort(paths->arrayfile, paths->sizefile)) {
		printf("error: failed to sort file array\n");
		return 1;
	} else if (paths->arraydir && ArraySort(paths->arraydir, paths->sizedir)) {
		printf("error: failed to sort dir array\n");
		return 1;
	}

	return 0;
}

int ArgumentsRead(int argc, char * argv[], Arguments * args) {
	if (!args || !argv) {
		printf("error: params empty\n");
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], ARG_BRIEF_DESCRIPTION)) {
			args->briefDescription = true;

		// if the first arg are flags
		} else if ((i == 1) && (argv[i][0] == '-')) {
			if (ArgumentsReadFlagsFromArg(argv[i], args)) {
				printf("error: couldn't read flags provided %s\n", argv[i]);
				return 1;
			}
		} else {
			if (PathListAddPath(&args->paths, argv[i])) {
				printf("error: couldn't add path %s\n", argv[i]);
				return 1;
			}
		}
	}

	// if no path was provided by user, we will
	// assume they want information from the current
	// directory
	if (PathListGetSize(&args->paths) == 0) {
		if (PathListAddPath(&args->paths, ".")) {
			printf("error: couldn't add current directory path\n");
			return 1;
		}
	}

	return PathListSort(&args->paths);
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

const char * StatModeTypeGetStringDescription(const char modtype) {
	switch (modtype) {
	case STAT_MOD_TYPE_BDEV: 	return STAT_MOD_TYPE_DESCRIPTION_BDEV;
	case STAT_MOD_TYPE_CDEV: 	return STAT_MOD_TYPE_DESCRIPTION_CDEV;
	case STAT_MOD_TYPE_DIR: 	return STAT_MOD_TYPE_DESCRIPTION_DIR;
	case STAT_MOD_TYPE_FIFO: 	return STAT_MOD_TYPE_DESCRIPTION_FIFO;
	case STAT_MOD_TYPE_SYMLINK: return STAT_MOD_TYPE_DESCRIPTION_SYMLINK;
	case STAT_MOD_TYPE_FILE: 	return STAT_MOD_TYPE_DESCRIPTION_FILE;
	case STAT_MOD_TYPE_SOCKET: 	return STAT_MOD_TYPE_DESCRIPTION_SOCKET;
	default: 					return STAT_MOD_TYPE_DESCRIPTION_UNKNOWN;
	}
}

/**
 * buf : buffer that will hold date
 * bufsize : size of the buf
 */
int TimeGetString(const BFTime time, char * buf, size_t bufsize) {
	if (!buf)
		return 1;

	BFDateTime dt;
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

// assumes out has at least PATH_MAX of bytes
// to write to
int GetPrintablePath(const PathQuery * in, char * out, const Arguments * args) {
	if (!in || !out)
		return 1;

	char buf[PATH_MAX];
	if (PathQueryGetPath(in, buf)) {
		printf("error: couldn't get path\n");
		return 1;
	}

	// if a path query doesn't have any parents, we can
	// assume the user explicitly provided this path.
	// Therefore we will return the entire (relative/absolute)
	// path
	if (PathQueryGetLevel(in) > 0) {
		const char * tmp = basename(buf);
		strncpy(out, tmp, PATH_MAX);
	} else {
		if (RemoveLeadingPeriodAndForwardSlashes(buf)) {
			printf("error: couldn't remove \"./\" from path '%s'\n", buf);
			return 1;
		}
		strncpy(out, buf, PATH_MAX);
	}


	return 0;
}

int PathQueryPrintPathBrief(const char * path, const char modetype, const mode_t m, BFTime modtime, const char * sizebuf, const char * color, const char * linkdesc) {
	char dt[64];
	TimeGetString(modtime, dt, sizeof(dt));
	printf("| %-1c-%03o %-21s %10s %s%s%s%s", modetype, m, dt, sizebuf,
			color,
			path,
			ANSI_COLOR_RESET,
			strlen(linkdesc) == 0 ? "" : linkdesc);

	printf("\n");

	return 0;
}

/**
 * assuming permissions follows = ||||||R|W|X|
 */
int PermissionsGetStringDescription(const mode_t permissions, char * buf, const size_t bufsize) {
	if (!buf) return 1;

	memset(buf, 0, bufsize);
	const char * arr[] = {"Executable", "Writable", "Readable"};
	const size_t size = sizeof(arr) / sizeof(arr[0]);

	for (int i = 0; i < size; i++) {
		if (permissions & (0x01 << i)) {
			if (strlen(buf)) {
				strcat(buf, ", ");
			}
			strcat(buf, arr[i]);
		}
	}
	
	return 0;
}

int PathQueryPrintPathDetail(
	const char * path,
	const char modetype,
	const mode_t m,
	BFTime modtime,
	BFTime accesstime,
	BFTime changetime,
	const char * sizebuf,
	const char * color,
	const char * linkdesc,
	uid_t owner
) {
	char res[2 << 8];
	char fullpath[PATH_MAX];
	realpath(path, fullpath);

	printf("Information for '%s'\n", path);
	printf("-----------------------------\n");

	struct passwd * pws = getpwuid(owner);
	printf("Owner: %s\n", pws->pw_name);

	printf("Type: %s\n", StatModeTypeGetStringDescription(modetype));
	printf("Full path: %s%s%s\n", color, fullpath, ANSI_COLOR_RESET);
	if (strlen(linkdesc) > 0)
		printf("Link: %s\n", linkdesc);
	
	printf("Size: %s\n", sizebuf);

	TimeGetString(modtime, res, sizeof(res));
	printf("Date Modified: %s\n", res);

	TimeGetString(accesstime, res, sizeof(res));
	printf("Date Access: %s\n", res);

	TimeGetString(changetime, res, sizeof(res));
	printf("Date Metadata Changed: %s\n", res);

	printf("Permissions:\n");

	// recall mode_t is an octal variable
	PermissionsGetStringDescription((m & S_IRWXU) >> (3 * 2), res, sizeof(res));
	printf("  Owner: %s\n", res);

	PermissionsGetStringDescription((m & S_IRWXG) >> (3 * 1), res, sizeof(res));
	printf("  Group: %s\n", res);

	PermissionsGetStringDescription((m & S_IRWXO) >> (3 * 0), res, sizeof(res));
	printf("  Other: %s\n", res);

	return 0;
}

int PathQueryPrintPath(const PathQuery * path, const Arguments * args) {
	if (!args || !path) return false;

	char p[PATH_MAX];
	PathQueryGetPath(path, p);

	// get info
	struct stat st;

	// see if file is a symlink
	if (lstat(p, &st) == -1) {
		printf("error: (path: %s) lstat %d\n", p, errno);
		return 1;
	}

	char buf[PATH_MAX];
	char linkdesc[PATH_MAX];
	memset(linkdesc, 0, sizeof(linkdesc));
	memset(buf, 0, sizeof(buf));

	// if the file isn't a symlink, default to 
	// reading it as a reg entry
	//
	// if link, then we will describe what
	// it is pointing to
	if (!S_ISLNK(st.st_mode)) {
		if (stat(p, &st) == -1) {
			printf("error: stat %d\n", errno);
			return 1;
		}
	} else {
		snprintf(linkdesc, PATH_MAX, " -> %s", 
				readlink(p, buf, sizeof(buf)) == -1 ? 
				"?" : buf);
	}

	// get size of entry
	// will not do recursion
	size_t size = st.st_size;
	char sizebuf[64];
	int error = BFByteGetString(size, 0, sizebuf);
	if (error)
		return error;

	// get permissions
	const mode_t m = st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

	// Get type of item this is at path
	const char modetype = StatGetModeType(&st);

	// color we will use to print
	const char * color = StatGetModeTypeColor(&st);

	// get printable path
	// making sure there are no redundant characters
	char item[PATH_MAX];
	if (GetPrintablePath(path, item, args)) {
		printf("error: couldn't get printable path\n");
		return 1;
	}

	// will only do it if the user asked for information for
	// ONE file
	bool shouldPrintInDetail = PathListGetSize(&args->paths) == 1 &&
		PathQueryGetLevel(path) == 0;

	if (shouldPrintInDetail) {
		return PathQueryPrintPathDetail(
			item,
			modetype, m,
			st.st_mtime,
			st.st_atime,
			st.st_ctime,
			sizebuf,
			color,
			strlen(linkdesc) == 0 ? "" : linkdesc,
			st.st_uid);
	} else {
		return PathQueryPrintPathBrief(
			item,
			modetype, m,
			st.st_mtime,
			sizebuf,
			color,
			strlen(linkdesc) == 0 ? "" : linkdesc);
	}
}

int PathQueryPrintDir(const PathQuery * dir, const Arguments * args) {
	if (!dir || !args) return 1;

	char p[PATH_MAX];
	PathQueryGetPath(dir, p);

	struct dirent ** namelist = NULL;
	int n = scandir(p, &namelist, NULL, alphasort);
	if (n == -1) {
		printf("error: couldn't scan dir %s\n", p);
		return 1;
	}

	bool shouldLabel = PathListGetSize(&args->paths) > 1;

	if (shouldLabel) {
		printf("\n%s:\n", p);
	}

	for (int i = 0; i < n; i++) {
		if (strcmp(namelist[i]->d_name, ".") && strcmp(namelist[i]->d_name, "..")) {
			char p[PATH_MAX];
			snprintf(p, PATH_MAX, "%s/%s", p, namelist[i]->d_name);

			PathQuery path;
			if (PathQueryCreateChild(dir, &path, namelist[i]->d_name)) {
				printf("error: couldn't create path query for %s\n", p);
				continue;
			}

			if (args->recursive && (namelist[i]->d_type == DT_DIR)) { // is dir
				// TODO: make some recursive thing
			} else {
				if (PathQueryPrintPath(&path, args)) {
					printf("error: path couldn't be worked on %s\n", p);
				}
			}

			PathQueryRelease(&path);
		}
		free(namelist[i]);
	}
	free(namelist);

	return 0;
}

int GetInfo(const Arguments * args) {
	if (!args) {
		printf("error: args param is empty\n");
		return 1;
	}

	for (int i = 0; i < PathListGetSize(&args->paths); i++) {
		char currpath[PATH_MAX];

		if (PathListGetPathAtIndex(&args->paths, i, currpath)) {
			printf("error: couldn't get path at index\n");
			continue;
		}

		PathQuery path;
		if (PathQueryCreate(&path, currpath)) {
			printf("error: couldn't create the path struct\n");
			continue;
		}

		int err = 0;
		if (PathQueryIsFile(&path)) {
			err = PathQueryPrintPath(&path, args);
		} else {
			err = PathQueryPrintDir(&path, args);
		}

		if (err) {
			printf("error: code - %d, path couldn't be worked on %s\n", err, currpath);
		}

		PathQueryRelease(&path);
	}

	return 0;
}

#ifdef TESTING

#include <bflibc/bftests.h>
#include <time.h>

int test_ArraySort(void) {
	UNIT_TEST_START;
	int result = 0;
	int max = 1;

	while (!result && max--) {
		char * arr[] = {"e", "d", "c", "b", "a"};
		size_t size = sizeof(arr) / sizeof(arr[0]);
		
		result = ArraySort(arr, size);
		if (result) continue;

		for (int i = 0; i < (size-1); i++) {
			if (strcmp(arr[i], arr[i + 1]) > 0) {
				result = 2;
				break;
			}
		}
	}

	UNIT_TEST_END(!result, result);
	return result;
}

int test_RemovingTrailingSlashesForRootPath(void) {
	UNIT_TEST_START;
	int result = 0;
	int max = PATH_MAX - 1;

	char buf[PATH_MAX];
	strncpy(buf, "/", PATH_MAX);
	while (!result && max--) {
		RemoveTrailingForwardSlashes(buf);
		if (strcmp(buf, "/")) {
			result = max;
		}

		// add another slash
		size_t s = strlen(buf);
		buf[s] = '/';
		buf[s+1] = '\0';
	}

	UNIT_TEST_END(!result, result);
	return result;
}

int test_RemovingTrailingSlashes(void) {
	UNIT_TEST_START;
	int result = 0;
	int max = 1;

	while (!result && max--) {
		char buf[PATH_MAX];
		snprintf(buf, PATH_MAX, "/hello/world/");
		RemoveTrailingForwardSlashes(buf);
		if (strcmp(buf, "/hello/world")) {
			result = max;
		}
	}

	UNIT_TEST_END(!result, result);
	return result;
}

int test_RemovingLeadingPeriodAndSlashes(void) {
	UNIT_TEST_START;
	int result = 0;
	int max = 2 << 8;

	while (!result && max--) {
		char buf[PATH_MAX];
		snprintf(buf, PATH_MAX, "./hello/world");
		RemoveLeadingPeriodAndForwardSlashes(buf);
		if (strcmp(buf, "hello/world")) {
			result = max;
		}
	}

	UNIT_TEST_END(!result, result);
	return result;

}

int TOOL_TEST(int argc, char ** argv) {
	int p = 0, f = 0;
	printf("TESTING: %s\n", argv[0]);

	LAUNCH_TEST(test_ArraySort, p, f);
	LAUNCH_TEST(test_RemovingTrailingSlashes, p, f);
	LAUNCH_TEST(test_RemovingLeadingPeriodAndSlashes, p, f);
	LAUNCH_TEST(test_RemovingTrailingSlashesForRootPath, p, f);

	PRINT_GRADE(p, f);

	return 0;
}

#endif // TESTING

