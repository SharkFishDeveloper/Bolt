
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lz4.h"
#include "gotoPrevCommitId.h"                
#include "commit.h"
#include "file_struct.h"
#include "stage_files.h"
#include "ht.h"

int checkIfPresentOnSameCommitAndBranch(char *commitId);
char *decompressTreeFile(const char *treeHashFullPath);
void parseTreeDataIntoStruct(char *data, F_STRUCT_ARRAY *array);


void gotoPreviousCommitId(char *commitId){
    // this is repeated work < just extracting current branch name >
    FILE *headpath = fopen("./.bolt/HEAD","r");
    char *commitNameRefs = malloc(256);
    if (!commitNameRefs) {
        perror("malloc failed");
        exit(1);
    }
    fgets(commitNameRefs, 256, headpath);
    char *last_slash = strrchr(commitNameRefs, '/');
    const char *branchName = last_slash + 1;
    // ---------------------
    int val = checkIfPresentOnSameCommitAndBranch(commitId);
    if(val == -1){
        printf("No commit present on branch: %s with commitId: %s",branchName,commitId);
        return;
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
    if(treeHash==NULL){
        return;
    }
    //-------------------------------
    char parenttreeHashDir[4];
    char parenttreeHashFile[38];
    char treeHashFullPath[100];
    strncpy(parenttreeHashDir, treeHash, 3);
    strncpy(parenttreeHashFile, treeHash + 3, 37);
    snprintf(treeHashFullPath,sizeof(treeHashFullPath),"./.bolt/obj/%s/%s",parenttreeHashDir,parenttreeHashFile);
    //-------------------------------
    
    char *data = decompressTreeFile(treeHashFullPath);
    F_STRUCT_ARRAY array = {NULL, 0, 10};
    parseTreeDataIntoStruct(data, &array);
    char sha1hashPathDir[4];
    char sha1hashPathFile[38];
    char sha1FullPath[80];
    for(int i = 0;i<array.count;i++){
        strncpy(sha1hashPathDir,array.files[i].sha1,3);
        strncpy(sha1hashPathFile, array.files[i].sha1 + 3, 37);
        snprintf(sha1FullPath,sizeof(sha1FullPath),"./.bolt/obj/%s/%s",sha1hashPathDir,sha1hashPathFile);
    }
    ht *hash_map = ht_create();
    printf("END");
    // F_STRUCT_ARRAY datafile = stageDirFiles(".",hash_map);
    // for (int i = 0; i < datafile.count; i++) {
    //     char *f = datafile.files[i].file;
    //     char *value = ht_get(hash_map, f);  // Retrieve the associated value
    //     printf("file: %s, value: %s\n", f, value);
    // }
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

 
char *decompressTreeFile(const char *treeHashFullPath) {
    FILE *treeFileRead = fopen(treeHashFullPath, "rb");
    if (!treeFileRead) {
        perror("Failed to open tree file");
        return NULL;
    }

    int compressedSize = 0;
    if (fread(&compressedSize, sizeof(int), 1, treeFileRead) != 1 || compressedSize <= 0) {
        fprintf(stderr, "Invalid compressed size\n");
        fclose(treeFileRead);
        return NULL;
    }

    char *compressedData = malloc(compressedSize);
    if (!compressedData) {
        perror("Memory allocation failed for compressed data");
        fclose(treeFileRead);
        return NULL;
    }

    if (fread(compressedData, 1, compressedSize, treeFileRead) != compressedSize) {
        fprintf(stderr, "Failed to read compressed data\n");
        free(compressedData);
        fclose(treeFileRead);
        return NULL;
    }
    fclose(treeFileRead);

    int decompressedSize = compressedSize * 2; // Start with double
    char *decompressedData = malloc(decompressedSize);
    if (!decompressedData) {
        perror("Memory allocation failed for decompressed data");
        free(compressedData);
        return NULL;
    }

    int actualSize = LZ4_decompress_safe(compressedData, decompressedData, compressedSize, decompressedSize);
    while (actualSize < 0) {
        decompressedSize *= 2;
        decompressedData = realloc(decompressedData, decompressedSize);
        if (!decompressedData) {
            perror("Memory reallocation failed");
            free(compressedData);
            return NULL;
        }
        actualSize = LZ4_decompress_safe(compressedData, decompressedData, compressedSize, decompressedSize);
    }

    free(compressedData);

    // Optional: add a null-terminator if you plan to treat it like a string
    decompressedData[actualSize] = '\0'; 

    return decompressedData; // Return decompressed data
}


void parseTreeDataIntoStruct(char *data, F_STRUCT_ARRAY *array) {
    if (array->files == NULL) {
        array->files = malloc(array->capacity * sizeof(F_STRUCT));
        if (array->files == NULL) {
            printf("Memory allocation failed\n");
            return;
        }
    }

    char *line = strtok(data, "\n"); // split by line
    while (line != NULL) {
        char path[512], typeStr[10], hash[50];
        int size1, size2;
        int fields = sscanf(line, "%511[^|]|%9[^|]|%49[^|]|%d|%d", path, typeStr, hash, &size1, &size2);

        if (fields == 5) {
            if (array->count >= array->capacity) {
                array->capacity = (array->capacity == 0) ? 10 : array->capacity * 2;
                array->files = realloc(array->files, array->capacity * sizeof(F_STRUCT));
                if (array->files == NULL) {
                    printf("Memory reallocation failed\n");
                    return;
                }
            }

            F_STRUCT *f = &array->files[array->count++];
            f->file = strdup(path);
            f->sha1 = strdup(hash);
            f->mode = size1; // or size2 depending on your logic

            if (strcmp(typeStr, "File") == 0) {
                f->type = FILE_TYPE_FILE;
            } else {
                f->type = FILE_TYPE_DIR;
            }
        } else {
            printf("Failed to parse line: %s\n", line);
        }

        line = strtok(NULL, "\n");
    }
}