
#include "readLastCommit.h"    
#include <dirent.h>         
#include <stdlib.h>
#include <string.h>
#include <stdio.h>   
                
char *readLastCommit(char *branchName) {
    char path[100];
    snprintf(path, sizeof(path), "./.bolt/logs/refs/heads/%s", branchName);
    printf("Trying to open: %s\n", path);

    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("Failed to open file");
        return NULL;
    }
    int foundLog = 0;
    char buffer[256]; // Adjust size as needed
    static char lastCommitId[50] = {0};
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strncmp(buffer, "commit:", 7) == 0){
            sscanf(buffer + 7, "%49s", lastCommitId); 
        }
    }
    printf("%s",lastCommitId);
    return lastCommitId;
}