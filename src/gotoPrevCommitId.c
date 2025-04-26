
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "gotoPrevCommitId.h"                
#include "commit.h"

int checkIfPresentOnSameCommitAndBranch(char *commitId);


void gotoPreviousCommitId(char *commitId){
    // this is repeated work
    FILE *headpath = fopen("./.bolt/HEAD","r");
    char *commitNameRefs =  malloc(60);
    fgets(commitNameRefs,256,headpath);
    char *last_slash = strrchr(commitNameRefs, '/');
    const char *branchName = last_slash + 1;
    // ---------------------
    int val = checkIfPresentOnSameCommitAndBranch(commitId);
    if(val == -1){
        printf("No commit present on branch: %s with commitId: %s",branchName,commitId);
    }else if(val == 1){
        return;
    }

    char treeDir[4];
    char fileName[38];
    char fullPath[100];
    strncpy(treeDir, commitId, 3);
    strncpy(fileName, commitId + 3, 37);
    snprintf(fullPath,sizeof(fullPath),"./.bolt/obj/%s/%s",treeDir,fileName);
    char *treeHash = extractParentCommitId(fullPath);
    if(strcmp(treeHash,"false")==0){
        return;
    }

    // now do checkout

}

int checkIfPresentOnSameCommitAndBranch(char *commitId){
    
    FILE *headpath = fopen("./.bolt/HEAD","r");
    char *commitNameRefs =  malloc(60);
    fgets(commitNameRefs,256,headpath);
    char *branchName = strrchr(commitNameRefs, ':');
    branchName+=2;
    branchName[strcspn(branchName, "\n")] = '\0';

    char refsfullCurrentBranchPath[60];
    char logsfullCurrentBranchPath[60];
    snprintf(refsfullCurrentBranchPath,sizeof(refsfullCurrentBranchPath),"./.bolt/%s",branchName);

    
    snprintf(logsfullCurrentBranchPath,sizeof(logsfullCurrentBranchPath),"./.bolt/logs/%s",branchName);
    
    FILE *logsPath = fopen(logsfullCurrentBranchPath,"r");
    char buffer[256]; // Adjust size as needed
    char currentCommitId[50];
    int foundLog = 0;
    while (fgets(buffer, sizeof(buffer), logsPath) != NULL) {
        if (strncmp(buffer, "commit:", 7) == 0){
            sscanf(buffer + 7, "%49s", currentCommitId); 
            if(strcmp(currentCommitId,commitId) == 0){
                foundLog = 1;
                break;
            }
        }
    }
    if(foundLog == 0){
        return -1;
    }
    fclose(logsPath);

    FILE *refsCurrentBranchFile = fopen(refsfullCurrentBranchPath,"r");
    char *refsChar =  malloc(100);
    fgets(refsChar,100,refsCurrentBranchFile);

    if(strcmp(refsChar,commitId) == 0){
        printf("You are already on %s",commitId);
        return 1;
    }
    return 0;
}

                