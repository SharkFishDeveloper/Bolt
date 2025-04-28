#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fileDirFunctions.h"                
                

void makeRecursivePath(const char *filePath) {
    char temp[512];
    char *p = NULL;
    size_t len;

    // Copy the file path to a temporary variable
    snprintf(temp, sizeof(temp), "%s", filePath);
    len = strlen(temp);

    // Find the last occurrence of '/'
    char *lastSlash = strrchr(temp, '/');
    if (lastSlash != NULL) {
        // Set the last part (file or folder) to '\0', leaving the directory part
        *lastSlash = '\0';
    }

    // Now create directories from the beginning up to the last part
    for (p = temp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (access(temp, F_OK) == -1) {
                mkdir(temp);  // Create the directory with appropriate permissions if it does not exist
            }
            *p = '/';
        }
    }
}