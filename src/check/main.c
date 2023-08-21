/**
 * author: Brando
 * date: 2/10/23
 */

#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <bflibc/bflibc.h>
#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#ifdef LINUX
#include <linux/limits.h>
#endif

// Arguments
#define kArgumentsMD5 "md5"
#define kArgumentsSHA1 "sha1"
#define kArgumentsSHA256 "sha256"
#define kArgumentsSHA512 "sha512"
#define kArgumentsBrieflyDescribeTool "--brief-description"

#define kArgumentsExpected "-e"

// Types
typedef struct {
	int fd; // file descriptor
	off_t * currSeek;
	BFChecksumTools * checksum;
	pthread_mutex_t * mutexRead;
	pthread_mutex_t * mutexHash;
	int error;
} ChecksumTools;

// Global variables
char filePath[PATH_MAX];
unsigned long long fileSize = 0;

// Constants
const int kThreadCount = 2;
size_t kBufferSizeRead = 4096 * 4096;

void help(int argc, char ** argv) {
	printf("usage: %s <checksum type> [ %s <expected hash> ] <file path>\n", basename(argv[0]), kArgumentsExpected);

	printf("\n");
	printf("Checksum types:\n");
	printf("  %s\n", kArgumentsMD5);
	printf("  %s\n", kArgumentsSHA1);
	printf("  %s\n", kArgumentsSHA256);
	printf("  %s\n", kArgumentsSHA512);
}

void BriefDescription() {
	printf("hash calculator & comparitor\n");
}

void * PThreadReadAndHash(void * _tools);
int CalculateChecksumForPath(const char * filePath, BFChecksumType checksumType, const char * expected);

int main(int argc, char ** argv) {
	int error = 0;
	BFChecksumType checksumType = kBFChecksumTypeUnknown;
	bool okayToContinue = true;

	if (argc < 3) {
		if (!strcmp(argv[1], kArgumentsBrieflyDescribeTool)) {
			BriefDescription();
			okayToContinue = false;
		} else {
			help(argc, argv);
			okayToContinue = false;
		}
	} else {
		if (!strcmp(argv[1], kArgumentsMD5)) {
			checksumType = kBFChecksumTypeMD5;
		} else if (!strcmp(argv[1], kArgumentsSHA1)) {
			checksumType = kBFChecksumTypeSHA1;
		} else if (!strcmp(argv[1], kArgumentsSHA256)) {
			checksumType = kBFChecksumTypeSHA256;
		} else if (!strcmp(argv[1], kArgumentsSHA512)) {
			checksumType = kBFChecksumTypeSHA512;
		} else {
			error = 8;
			BFErrorPrint("Please provide a checksum type");
		}
	}

	if (okayToContinue) {
		// Read other arguments
		const char * expected = NULL;
		if (error == 0) {
			for (int i = 2; i < (argc - 1); i++) {
				// user provied expected checksum
				if (!strcmp(argv[i], kArgumentsExpected)) {
					i++;
					expected = argv[i];
				}
			}
		}

		// Last one will always be path
		if (error == 0) {
			strcpy(filePath, argv[argc - 1]);
			if (!BFFileSystemPathIsFile(filePath)) {
				error = 2;
			}
		}

		if (error == 0) {
			error = CalculateChecksumForPath(filePath, checksumType, expected);
		}
	}

	return error;
}

int CalculateChecksumForPath(const char * filePath, BFChecksumType checksumType, const char * expected) {
	int error = 0;
	BFChecksumTools ctools = {0};
	pthread_mutex_t mutexRead, mutexHash;

	// Init checksum tools
	error = BFChecksumCreate(&ctools, checksumType);

	if (error == 0) {
		if (pthread_mutex_init(&mutexRead, 0)) {
			error = 4;
			BFDLog("Could not initialize pthread mutex for read");
		} else if (pthread_mutex_init(&mutexHash, 0)) {
			error = 5;
			BFDLog("Could not initialize pthread mutex for hash");
		}
	}

	if (error == 0) {
		fileSize = BFFileSystemFileGetSizeUsed(filePath, 0, &error);
	}

	off_t currSeek = 0;
	int i = 0;
	pthread_t threads[kThreadCount];
	ChecksumTools checksumTools[kThreadCount];
	while ((error == 0) && (i < kThreadCount)) {
		if ((checksumTools[i].fd = open(filePath, O_RDONLY)) == -1) {
			error = 3;
		} else {
			checksumTools[i].currSeek = &currSeek;
			checksumTools[i].error = 0;
			checksumTools[i].mutexRead = &mutexRead;
			checksumTools[i].mutexHash = &mutexHash;
			checksumTools[i].checksum = &ctools;
		}

		if ((error == 0) && pthread_create(&threads[i], 0, PThreadReadAndHash, &checksumTools[i])) {
			error = 8;
			BFDLog("Could not launch pthread");
		}

		i++;
	}

	i = 0;
	while ((error == 0) && (i < kThreadCount)) {
		if (pthread_join(threads[i], 0)) {
			error = 6;
		} else {
			if (checksumTools[i].error) {
				BFErrorPrint("Error occurred in thread %d: %d", i, checksumTools[i].error);
			}
			close(checksumTools[i].fd);
		}

		i++;
	}

	if (error == 0) {
		error = BFChecksumFinal(&ctools);
	}

	char * hashStr = 0;
	if (error == 0) {
		hashStr = (char *) malloc(BFChecksumGetDigestStringLength(&ctools) + 1);
		if (hashStr == NULL) {
			error = 7;
		} else {
			error = BFChecksumGetDigestString(&ctools, hashStr);
		}
	}

	// Print
	if (error == 0) {
		printf("%s", hashStr);

		if (expected) {
			printf(" [ ");
			if (strcmp(hashStr, expected)) {
				printf("\x1B[31m" "fail" "\033[0m");
			} else {
				printf("\x1B[32m" "pass" "\033[0m");
			}
			printf(" ]");
		}

		printf("\n");
	}

	BFChecksumDestroy(&ctools);
	BFFree(hashStr);

	return error;
}

void * PThreadReadAndHash(void * _tools) {
	bool finished = false;
	ChecksumTools * tools = (ChecksumTools *) _tools;
	size_t bufferSize = kBufferSizeRead;
	void * buf = malloc(bufferSize);

	do {
		pthread_mutex_lock(tools->mutexRead);
	
		// READ

		size_t upcoming = fileSize - *tools->currSeek;
		if (upcoming == 0) { // no more bytes to read
			finished = true;
		} else if (upcoming < bufferSize) {
			bufferSize = upcoming;
		}

		if (!finished) {
			// Get current position
			if (lseek(tools->fd, *tools->currSeek, SEEK_SET) == -1) {
				tools->error = 52;
			
			// Read file
			} else if (read(tools->fd, buf, bufferSize) != bufferSize) {
				tools->error = 50;

			// Save current position for next thread
			} else if ((*tools->currSeek = lseek(tools->fd, 0, SEEK_CUR)) == -1) {
				tools->error = 51;
			}
		}

		pthread_mutex_lock(tools->mutexHash);
		pthread_mutex_unlock(tools->mutexRead);

		// HASH
		
		if (!finished && (tools->error == 0)) {
			tools->error = BFChecksumUpdate(tools->checksum, buf, bufferSize);
		}
		
		pthread_mutex_unlock(tools->mutexHash);
	} while (!finished && (tools->error == 0));

	BFFree(buf);
	return 0;
}

