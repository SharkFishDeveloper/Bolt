#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fileDirFunctions.h"                
                

void makeRecursivePath(const char *filePath) {
    char temp[512];
    size_t len = strlen(filePath);

    if (len >= sizeof(temp)) return;  // Avoid overflow

    snprintf(temp, sizeof(temp), "%s", filePath);

    // Remove the filename or last path segment if it's a file
    char *lastSlash = strrchr(temp, '/');
    if (lastSlash) {
        *lastSlash = '\0';
    } else {
        return;  // No directory to create
    }

    for (char *p = temp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';

            if (access(temp, F_OK) == -1) {
                mkdir(temp);  // Recursively create each directory
            }

            *p = '/';
        }
    }

    // Finally, make the full directory if it doesn't exist
    if (access(temp, F_OK) == -1) {
        mkdir(temp);
    }
}