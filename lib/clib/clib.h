/**
 * author: Brando
 * date: 6/2/22
 */

#include <stdbool.h>

// MACROS

#define PATH_MAX 4096

#define KILOBYTE 1024
#define MEGABYTE (long long) (KILOBYTE * 1024)
#define GIGABYTE (long long) (MEGABYTE * 1024)
#define TERABYTE (long long) (GIGABYTE * 1024)

/**
 * Calculates the directory size recursively
 */
unsigned long long CalculateDirectorySize(const char * path, int * error);

/**
 * Calculates the file size of the path  
 */
unsigned long long CalculateFileSize(const char * path, int * error);

/**
 * Prints std error
 */
void Error(const char * format, ...);

/**
 * Tests if path exists
 */
bool PathExists(const char * path);

/**
 * Tests if path is a file
 */
bool IsFile(const char * path);

/**
 * Tests if path is a directory
 */
bool IsDirectory(const char * path);

/**
 * Tests if the path is a symbolic link 
 */
bool IsSymbolicLink(const char * path);

/**
 * Creates a string that represents the byteSize
 *
 * outStr needs to be a valid string buffer that can accept at least 10 bytes of data
 */
int GetByteStringRepresentation(unsigned long long byteSize, char * outStr);

/**
 * Sweeps the array to see of it contains the string specified by element
 */
bool DoesStringArrayContain(char ** strArr, int arrSize, const char * element);

/**
 * Creates a copy of string
 */
char * CopyString(const char * string, int * err);

