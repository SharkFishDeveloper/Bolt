
#include "gotToBranch.h"     
#include "readLastCommit.h"  
#include "gotoPrevCommitId.h"      
#include "checkHead.h"                
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>

int gotToBranch(char *branchname){
    char *currentHead = headPath();
    if(strcmp(branchname,currentHead)==0){
        printf("Already present on %s",currentHead);
        return -1;
    }

    char newBranchRefpath[200];
    snprintf(newBranchRefpath,sizeof(newBranchRefpath),"./.bolt/refs/heads/%s",branchname);
    if(access(newBranchRefpath,F_OK)!=0){
        printf("Branch: %s is not present",branchname);
        return -1;
    }

    char refsHeadFile[100];
    snprintf(refsHeadFile,sizeof(refsHeadFile),"ref: refs/heads/%s",branchname);

    char *lastCommitID = readLastCommit(branchname);
    gotoPreviousCommitId(lastCommitID,1);

    FILE *HEADfile = fopen("./.bolt/HEAD","w"); 
    fprintf(HEADfile, "%s", refsHeadFile);
    fclose(HEADfile); // <- Write in HEAD

    return 1;
}