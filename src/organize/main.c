/**
 * author: Brando
 * date: 1/19/23
 *
 * todo:
 * 	- rename exe
 */

#include <stdio.h>
#include <bflibc/bflibc.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <libgen.h>
#include <limits.h>

#ifdef LINUX
#include <linux/limits.h>
#endif

#define kArgumentsBriefDescription "--brief-description"
#define kArgumentsSource "-s"
#define kArgumentsDestination "-d"

/// Organize types
#define kOrganizeTypeByMonth "month"

char kToolname[PATH_MAX];
char kOrganizeTypename[2 << 4];
char kSourcePath[PATH_MAX];
char kDestinationPath[PATH_MAX];

int Organize();

void help() {
	printf("usage: %s <organize type> %s <source path> %s <output path>\n", kToolname, kArgumentsSource, kArgumentsDestination);

	printf("\nOrganize types:\n");
	printf("  - %s\n", kOrganizeTypeByMonth);

	printf("\nDiscussion:\n");
	printf("  I wrote this tool for the sole purpose to organize my photos into my personal\n");
	printf("  library.  My workflow is typically dumping all my picture files into one folder,\n");
	printf("  all files no subfolders, then I just run this tool to organize each file into\n");
	printf("  their respective folder based on the creation month (for example).\n");

	printf("\n");
}

void briefDescription() {
	printf("organizes input files into an output directory\n");
}

int main(int argc, char ** argv) {
	int error = 0;
	int i = 0;
	char buf[PATH_MAX];

	strcpy(kToolname, argv[0]);

	if (argc < 2) {
		error = 3;
		help();
	} else if (BFArrayStringContainsString(argv, argc, kArgumentsBriefDescription)) {
		error = 3;
		briefDescription();
	} else {
		strcpy(kOrganizeTypename, argv[1]);
	}

	// Get destination path
	if (error == 0) {
		i = BFArrayStringGetIndexForString(argv, argc, kArgumentsDestination);

		if (i == -1) {
			error = 2;
		} else if ((i + 1) < argc) {
			if (realpath(argv[i + 1], buf) == NULL) {
				error = 2;
			} else {
				strcpy(kDestinationPath, buf);
			}
		} else {
			error = 2;
		}

		if (error) BFErrorPrint("No destination path provided");
	}

	if (error == 0) {
		// Get source path
		i = BFArrayStringGetIndexForString(argv, argc, kArgumentsSource);
		if (i == -1) {
			error = 1;
		} else if ((i + 1) < argc) {
			if (realpath(argv[i + 1], buf) == NULL) {
				error = 2;
			} else {
				strcpy(kSourcePath, buf);
			}
		} else {
			error = 1;
		}

		if (error)
			BFErrorPrint("No source path provided");
	}

	if (error == 0) {
		error = Organize();
	}


	return error;
}

int SetUniqueFilePath(char * filepath) {
	int result = 0;
	char bname[PATH_MAX];
	char dname[PATH_MAX];
	char ext[PATH_MAX];
	char temppath[PATH_MAX];

	strcpy(temppath, filepath); // copy full filepath

	char * tmp = dirname(temppath); // get path to file
	strcpy(dname, tmp);

	strcpy(temppath, filepath); // make sure we have a fresh copy

	// get name w/o ext
	result = BFFileSystemPathGetName(temppath, bname);

	// get ext
	if (result == 0) {
		result = BFFileSystemPathGetExtension(temppath, ext);
	}

	// see if file path already exists, if so let's add a number
	// to the basename
	if (result == 0) {
		int i = 1;
		char newpath[PATH_MAX];
		strcpy(newpath, filepath);
		while (BFFileSystemPathIsFile(newpath)) {
			sprintf(newpath, "%s/%s_%d.%s", dname, bname, i, ext);
			i++;
		}

		strcpy(filepath, newpath);
	}

	return result;
}

int MoveByMonth(const char * source, const char * destination) {
	int result = 0;
	char buf[PATH_MAX];
	char bname[PATH_MAX];
	struct stat st;

	if (stat(source, &st) == -1) {
		result = 7;
	}

	if (result == 0) {
#ifdef MACOS
		struct tm * tm = localtime(&st.st_birthtimespec.tv_sec);
#else
		struct tm * tm = localtime(&st.st_ctim.tv_sec);
#endif

		strcpy(bname, source);
		char * tmp = basename(bname);
		strcpy(bname, tmp);
		sprintf(buf, "%s/%d-%02d", destination, tm->tm_year + 1900, tm->tm_mon + 1);

		if (!BFFileSystemPathIsDirectory(buf)) {
			if (mkdir(buf, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
				result = 10;
			}
		}
	}

	// Check if path exists
	if (result == 0) {
		sprintf(buf, "%s/%s", buf, bname);
		result = SetUniqueFilePath(buf);
	}
	
	if (result == 0) {
		printf("%s -> %s\n", source, buf);
		if (rename(source, buf) != 0) {
			result = 12;
		}
	}

	return result;
}

int Move(const char * source, const char * destination) {
	int result = 0;

	if (!strcmp(kOrganizeTypename, kOrganizeTypeByMonth)) {
		result = MoveByMonth(source, destination);
	} else {
		result = 8;
	}

	return result;
}

int OrganizeDirectory() {
	int result = 0;
	// We assume source is directory already
	DIR * dir = opendir(kSourcePath);
	struct dirent * ent = NULL;
	char buf[PATH_MAX];

	if (dir == NULL) {
		result = 9;
	}

	while (!result && ((ent = readdir(dir)) != NULL)) {
		if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
			sprintf(buf, "%s/%s", kSourcePath, ent->d_name);

			if (BFFileSystemPathIsFile(buf))
				result = Move(buf, kDestinationPath);
			else {
				BFErrorPrint("'%s' is not a file", buf);
			}
		}
	}

	if (dir)
		closedir(dir);

	return result;
}

int Organize() {
	int result = 0;

	printf("Organize type: %s\n", kOrganizeTypename);
	printf("source: %s\n", kSourcePath);
	printf("destination: %s\n", kDestinationPath);

	// Make sure the paths exist
	if (!BFFileSystemPathExists(kDestinationPath)) {
		result = 4;
		BFErrorPrint("Path %s does not exist", kDestinationPath);
	}

	if (BFFileSystemPathIsFile(kSourcePath)) {
		result = Move(kSourcePath, kDestinationPath);
	} else if (BFFileSystemPathIsDirectory(kSourcePath)) {
		result = OrganizeDirectory();
	} else {
		result = 5;
	}

	return result;
}

