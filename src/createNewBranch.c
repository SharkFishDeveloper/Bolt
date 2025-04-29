
#include "createNewBranch.h"                
#include "checkHead.h"                
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>

int createNewBranch(char * branchname){
    char *currentHead = headPath();
    if(strcmp(branchname,currentHead)==0){
        printf("Already present on %s",currentHead);
        return -1;
    }
    char parentLatestCommitRefsPath[200];
    snprintf(parentLatestCommitRefsPath,sizeof(parentLatestCommitRefsPath),"./.bolt/refs/heads/%s",currentHead);

    char newBranchRefpath[200];
    snprintf(newBranchRefpath,sizeof(newBranchRefpath),"./.bolt/refs/heads/%s",branchname);
    if(access(newBranchRefpath,F_OK)==0){
        printf("Branch: %s already present",branchname);
        return -1;
    }

    char parentLatestCommitId[60];
    FILE *fp = fopen(parentLatestCommitRefsPath, "r");
    fgets(parentLatestCommitId,100,fp);
    fclose(fp);

    FILE *newbranchRefs = fopen(newBranchRefpath,"w");
    fprintf(newbranchRefs, "%s", parentLatestCommitId);
    fclose(newbranchRefs);

    char refsHeadFile[100];
    snprintf(refsHeadFile,sizeof(refsHeadFile),"ref: refs/heads/%s",branchname);
    FILE *HEADfile = fopen("./.bolt/HEAD","w"); 
    fprintf(HEADfile, "%s", refsHeadFile);
    fclose(HEADfile);

    char parentLogsPath[100];
    snprintf(parentLogsPath,sizeof(parentLogsPath),"./.bolt/logs/refs/heads/%s",currentHead);

    char childLogsPath[100];
    snprintf(childLogsPath,sizeof(childLogsPath),"./.bolt/logs/refs/heads/%s",branchname);
    FILE *src = fopen(parentLogsPath, "r");
    if (!src) {
        perror("Error opening parent log file");
        return -1;
    }

    FILE *dest = fopen(childLogsPath, "w");
    if (!dest) {
        perror("Error opening child log file");
        fclose(src);
        return -1;
    }

    char buffer[1024];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytesRead, dest);
    }

    fclose(src);
    fclose(dest);
    return 1;


}