unsigned long long CalculateDirectorySize(const char * path, int * err) {
	unsigned long long result = 0;
	int error = 0;
	DIR * dir = 0;
	struct dirent * sdirent = 0;
	char tempPath[PATH_MAX];
	unsigned long long size = 0;

	dir = opendir(path);
	
	if (dir == 0) {
		error = 1;
		Error("Could not open directory: '%s'", path);
	} else {
		// Go through each item in this directory
		while ((sdirent = readdir(dir)) && !error) {
			// We do not want to calculate the size of parent or current dir
			if (strcmp(sdirent->d_name, "..") && strcmp(sdirent->d_name, ".")) {
				// We want the full directory path
				sprintf(tempPath, "%s/%s", path, sdirent->d_name);

				size = 0;

				// If we are working with a directory, then we need 
				// too recursively call this function again 
				//
				// If path is not a diretory or a path then we will 
				// do nothing 
				//
				// We will also be ignoring symbolic links 
				if (IsSymbolicLink(tempPath)) {
					size = 0;
				} else if (IsDirectory(tempPath)) {
					size = CalculateDirectorySize(tempPath, &error);
				
				// Otherwise, we will just get the size of this path 
				} else if (IsFile(tempPath)) {
					size = CalculateFileSize(tempPath, &error);
				}

				if (!error) {
					result += size;
				}
			}
		}

		if (closedir(dir)) {
			error = !error ? 1 : error; 
			Error("Could not close directory: '%s'", path);
		}
