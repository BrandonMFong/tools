/**
 * author: Brando
 * date: 6/2/22
 */

#include <stdbool.h>

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

