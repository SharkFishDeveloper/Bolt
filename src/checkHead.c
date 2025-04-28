#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "checkHead.h"                
                
char *headPath() {
    static char branchName[256];  // static so it survives after function ends
    char headsPath[60];
    snprintf(headsPath, sizeof(headsPath), "./.bolt/HEAD");
    FILE *headPathFileName = fopen(headsPath, "r");
    if (!headPathFileName) {
        perror("Unable to open HEAD file");
        return NULL;
    }
    char commitNameRefs[256];
    if (fgets(commitNameRefs, sizeof(commitNameRefs), headPathFileName) == NULL) {
        fclose(headPathFileName);
        perror("Unable to read HEAD file");
        return NULL;
    }
    fclose(headPathFileName);
    char *lastSlash = strrchr(commitNameRefs, '/');
    if (!lastSlash) {
        printf("Invalid HEAD format (no slash)\n");
        return NULL;
    }
    lastSlash++; // move past '/'
    snprintf(branchName, sizeof(branchName), "%s", lastSlash);
    branchName[strcspn(branchName, "\n")] = 0;
    return branchName;
}

char *refsheadPath() {
    static char branchName[256];
    char headsPath[60];
    snprintf(headsPath, sizeof(headsPath), "./.bolt/HEAD");
    FILE *headPathFileName = fopen(headsPath, "r");
    if (!headPathFileName) {
        perror("Unable to open the HEAD file");
        return NULL;
    }
    char commitNameRefs[256];
    fgets(commitNameRefs, sizeof(commitNameRefs), headPathFileName);
    char *colonPos = strchr(commitNameRefs, ':');
    if (colonPos) {
        colonPos++;  // Move to the character after the colon
        while (*colonPos == ' ') {
            colonPos++;  // Skip any spaces after the colon
        }
        snprintf(branchName, sizeof(branchName), "%s", colonPos);
    }
    fclose(headPathFileName);
    return branchName;
}

int checkIfCommitExistsInBranch(){
    char *currentHead = headPath();  // <- main
    char *refshead = refsheadPath(); // <- refs/heads/main
    char completeRefsPath[200];
    snprintf(completeRefsPath,sizeof(completeRefsPath),"./.bolt/%s",refshead);
    FILE *refFile = fopen(completeRefsPath, "r");
    if (!refFile) {
        perror("Unable to open refs file");
        return -1;
    }

    char currentCommit[100];
    if (!fgets(currentCommit, sizeof(currentCommit), refFile)) {
        perror("Unable to read current commit");
        fclose(refFile);
        return -1;
    }
    fclose(refFile);
    currentCommit[strcspn(currentCommit, "\n")] = 0; 

    char logsPath[300];
    snprintf(logsPath,sizeof(logsPath),"./.bolt/logs/refs/heads/%s",currentHead);
    FILE *logsFile = fopen(logsPath, "r");
    if (!logsFile) {
        perror("Unable to open logs file");
        return -1;
    }

    // Seek to near end
    fseek(logsFile, -1000, SEEK_END);  // Go 1000 bytes before end (safe for small logs)
    
    char buffer[1100] = {0};
    fread(buffer, 1, sizeof(buffer) - 1, logsFile); 
    fclose(logsFile);

    // Now find the LAST "commit:" in buffer
    char *lastCommitLine = NULL;
    char *ptr = buffer;
    while ((ptr = strstr(ptr, "commit:")) != NULL) {
        lastCommitLine = ptr;
        ptr++;  // Move ahead to find next occurrence
    }

    if (!lastCommitLine) {
        printf("No commit found in logs!\n");
        return -1;
    }

    char lastCommit[100];
    sscanf(lastCommitLine + 7, "%s", lastCommit);  // Skip "commit:" and read the commit id
    // printf("currentCommit %s",currentCommit);
    if (strcmp(currentCommit, lastCommit) == 0) {
        return 1; // On latest commit
    } else {
        return 0; // Not on latest commit
    }
}

