
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include "lz4.h"
#include "gotoPrevCommitId.h"                
#include "commit.h"
#include "file_struct.h"
#include "stage_files.h"
#include "ht.h"
#include "stage.h"
#include "decompressLz4.h"

int checkIfPresentOnSameCommitAndBranch(char *commitId);
char *decompressTreeFile(const char *treeHashFullPath);
void parseTreeDataIntoStruct(char *data, F_STRUCT_ARRAY *array,ht *map);
void createNestedDir(const char *path);
void removeAllDirs(const char *dirPath);
int directoryExists(const char *path);


void gotoPreviousCommitId(char *commitId){
    // this is repeated work < just extracting current branch name >
    FILE *headpath = fopen("./.bolt/HEAD", "r");
    if (!headpath) {
        perror("Failed to open HEAD file");
        return;
    }

    char commitNameRefs[256];
    if (!fgets(commitNameRefs, sizeof(commitNameRefs), headpath)) {
        perror("Failed to read from HEAD file");
        fclose(headpath);
        return;
    }
    fclose(headpath);
    // Extract branch name from commitNameRefs
    char *branchName = strrchr(commitNameRefs, '/') + 1;

    // Check if the commit is present on the same branch
    if (checkIfPresentOnSameCommitAndBranch(commitId) != 0) {
        printf("No commit present on branch: %s with commitId: %s\n", branchName, commitId);
        return;
    }
    removeAllDirs("c1/c2/c3");
    return;
    // Build the path for the commit object and extract parent commit ID
    char fullPath[100];
    snprintf(fullPath, sizeof(fullPath), "./.bolt/obj/%3.3s/%s", commitId, commitId + 3);

    char *treeHash = extractParentCommitId(fullPath);
    if (!treeHash) return;
    snprintf(fullPath, sizeof(fullPath), "./.bolt/obj/%3.3s/%s", treeHash, treeHash + 3);

    // part 3 -----------------------------------------------------------------------------------------
    char *data = decompressTreeFile(fullPath);

    F_STRUCT_ARRAY staging_array = {NULL, 0, 10};

    ht *hash_map_stage_era = ht_create();//<- this hashmap stores staged area path
    parseTreeDataIntoStruct(data, &staging_array,hash_map_stage_era);
    // stage(&staging_array);
    return;

    char sha1hashPathDir[4];
    char sha1hashPathFile[38];
    char sha1FullPath[80];

    ht *hash_map = ht_create();//<- this hashmap stores current dir path
    F_STRUCT_ARRAY currentDirFiles = stageDirFiles(".",hash_map);

    for(int i = 0;i<staging_array.count;i++){
        char *stagedSha1 = staging_array.files[i].sha1;
        char *stagedContentSha1 = ht_get(hash_map,staging_array.files[i].file);
        if(stagedContentSha1 != NULL && strcmp(stagedContentSha1,stagedSha1)!=0){
            if(staging_array.files[i].type == FILE_TYPE_DIR){
                // make a dir
                createNestedDir(staging_array.files[i].file);
            }
            else if(staging_array.files[i].type == FILE_TYPE_FILE){
                char path[256];
                snprintf(path, sizeof(path), "./.bolt/obj/%.3s/%s", stagedSha1, stagedSha1 + 3);
                int decompressedLength = 0;
                char *data = decompressFile(path, &decompressedLength);
                printf("DATA -> %s ",data);
            }
        }
    }
    
    for (int i = 0; i < currentDirFiles.count; i++) {
        char *stagedContentSha1 = ht_get(hash_map_stage_era,currentDirFiles.files[i].file);
        if(stagedContentSha1 == NULL){
            if(currentDirFiles.files[i].type == FILE_TYPE_DIR){
                removeAllDirs(currentDirFiles.files[i].file);
            }
            else if(currentDirFiles.files[i].type == FILE_TYPE_FILE){
                remove(currentDirFiles.files[i].file);
            }
        }        
    }
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


void parseTreeDataIntoStruct(char *data, F_STRUCT_ARRAY *array,ht *map) {
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
            char *hash_copy = strdup(hash);
            ht_set(map, path, (void*)hash_copy);
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

void createNestedDir(const char *path) {
    char *tempPath = strdup(path); // Create a mutable copy of the path
    if (tempPath == NULL) {
        perror("strdup");
        return;
    }

    char *p = tempPath;
    while (*p != '\0') {
        if (*p == '/') {
            *p = '\0'; // Null-terminate at the slash to get the parent directory

            // If the parent directory doesn't exist, create it
            if (!directoryExists(tempPath)) {
                if (mkdir(tempPath) == -1) {
                    if (errno != EEXIST) { // Ignore if the directory already exists
                        perror("mkdir");
                        free(tempPath);
                        return;
                    }
                }
                printf("Created directory: %s\n", tempPath);
            }
            *p = '/'; // Restore the slash for the next iteration
        }
        p++;
    }

    // Create the final directory
    if (!directoryExists(tempPath)) {
        if (mkdir(tempPath) == -1) {
            if (errno != EEXIST) {
                perror("mkdir");
            }
        } else {
            printf("Created directory: %s\n", tempPath);
        }
    }

    free(tempPath);
}

int directoryExists(const char *path) {
    struct stat info;
    if (stat(path, &info) == 0 && S_ISDIR(info.st_mode)) {
        return 1; // Directory exists
    }
    return 0; // Directory does not exist or there was an error
}

void removeAllDirs(const char *path) {
    char *tempPath = strdup(path);
    if (tempPath == NULL) {
        perror("strdup");
        return;
    }

    char *p = tempPath + strlen(tempPath); // Start from the end of the path

    while (p > tempPath) {
        if (*p == '/') {
            *p = '\0'; // Null-terminate to get the current directory level
            if (rmdir(tempPath) == -1) {
                if (errno == ENOTEMPTY) {
                    printf("Directory '%s' is not empty, cannot remove.\n", tempPath);
                    break; // Stop if a directory is not empty
                } else if (errno == ENOENT) {
                    // Directory doesn't exist, which is fine, continue to the parent
                } else {
                    perror("rmdir");
                    break; // Stop on other errors
                }
            } else {
                printf("Removed directory: %s\n", tempPath);
            }
            *p = '/'; // Restore the slash for the next iteration
        }
        p--;
    }

    // Finally, try to remove the topmost directory
    if (rmdir(tempPath) == -1) {
        if (errno == ENOTEMPTY) {
            printf("Directory '%s' is not empty, cannot remove.\n", tempPath);
        } else if (errno == ENOENT) {
            // Topmost directory doesn't exist, which is fine
        } else {
            perror("rmdir");
        }
    } else {
        printf("Removed directory: %s\n", tempPath);
    }

    free(tempPath);
}
