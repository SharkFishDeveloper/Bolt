#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stage_files.h"
#include "init.h"
#include "file_struct.h"
#include "findSHA1.h"
#include "stage.h"
#include "status.h"
#include "stage_file_struct.h"
#include "map.h"

typedef struct {
    F_STRUCT data;
} StagedMapValue;

STAGE_FILE_STRUCT status(){
    hashmap* stagedMap = hashmap_create();
    hashmap* dirListMap = hashmap_create();
    F_STRUCT_ARRAY fileList = stageDirFiles(".",NULL);

    F_STRUCT_ARRAY stagedFiles = read_index(".bolt/index.bin");

    STAGE_FILE_STRUCT result = {
        .addedFileCount = 0,
        .modedFileCount = 0,
        .deletedFileCount = 0,
        .addedFileCapacity = 10,
        .modedFileCapacity = 10,
        .deletedFileCapacity = 10
    };

    result.addedFiles   = malloc(result.addedFileCapacity   * sizeof(char *));
    result.modedFiles   = malloc(result.modedFileCapacity   * sizeof(char *));
    result.deletedFiles = malloc(result.deletedFileCapacity * sizeof(char *));

    for (int i = 0; i < stagedFiles.count; i++) {
        // for staged files
        // printf("staged-> %s length %d",stagedFiles.files[i].file,strlen(stagedFiles.files[i].sha1));
        StagedMapValue* val = malloc(sizeof(StagedMapValue));
        if (!val) {
            exit(EXIT_FAILURE);
        }
        val->data = stagedFiles.files[i];
        hashmap_set(stagedMap, stagedFiles.files[i].file, strlen(stagedFiles.files[i].file), (uintptr_t)val);
    }
    for (int i = 0; i < fileList.count; i++) {
        // for working directory files
        StagedMapValue* val = malloc(sizeof(StagedMapValue));
        if (!val) {
            exit(EXIT_FAILURE);
        }
        val->data = fileList.files[i];
        hashmap_set(dirListMap, fileList.files[i].file, strlen(fileList.files[i].file), (uintptr_t)val);
    }

    for (int i = 0; i < fileList.count; i++) {
        F_STRUCT current = fileList.files[i];
        uintptr_t val_ptr;
        int exists = hashmap_get(stagedMap, current.file, strlen(current.file), &val_ptr);
        
        if (exists == 0) {
            // New file found
            if (result.addedFileCount == result.addedFileCapacity) {
                result.addedFileCapacity *= 2;
                result.addedFiles = realloc(result.addedFiles, result.addedFileCapacity * sizeof(char *));
            }
            result.addedFiles[result.addedFileCount++] = strdup(current.file);
        } else {
            // Check if it's modified
            StagedMapValue* val = (StagedMapValue*)val_ptr;
            // printf("current.sha1-> %s val->data.sha1 %s $$$ \n",current.sha1,val->data.sha1);
            // if(strcmp(current.sha1,val->data.sha1) == 0){
            //     printf("EQUAL");
            // }
            // if ((current.type == FILE_TYPE_FILE && val->data.type == FILE_TYPE_FILE) &&
            //     memcmp(current.sha1, val->data.sha1, 20) != 0) {
            if ((current.type == FILE_TYPE_FILE && val->data.type == FILE_TYPE_FILE) &&
                strcmp(current.sha1,val->data.sha1) != 0) {
                // Modified content
                if (result.modedFileCount == result.modedFileCapacity) {
                    result.modedFileCapacity *= 2;
                    result.modedFiles = realloc(result.modedFiles, result.modedFileCapacity * sizeof(char *));
                }
                result.modedFiles[result.modedFileCount++] = strdup(current.file);
            }  
        }
    }
    for(int i = 0; i <stagedFiles.count;i++){
        F_STRUCT current = stagedFiles.files[i];
        uintptr_t val_ptr;
        if (hashmap_get(dirListMap, current.file, strlen(current.file), &val_ptr) == 0) {
            // deleted file
            if (result.deletedFileCount == result.deletedFileCapacity) {
                result.deletedFileCapacity *= 2;
                result.deletedFiles = realloc(result.deletedFiles, result.deletedFileCapacity * sizeof(char *));
            }
            result.deletedFiles[result.deletedFileCount++] = strdup(current.file);
        }
    }
    return result;
}
