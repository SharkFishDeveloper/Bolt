#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>  
#include "commit.h"                
#include "file_struct.h"
#include "sha1ToHex.h"

void commit(F_STRUCT_ARRAY *stagedFiles,char* message){
    if(stagedFiles->count == 0){
        printf("\x1b[33mPlease add first using - <bolt add>\x1b[0m\n");
        return;
    }
    for(int i = 0 ;i<stagedFiles->count;i++){
        //check if index and previos commit blob files are same, to avoid duplication
        if(stagedFiles->files[i].type == FILE_TYPE_FILE){
            char *hash = sha1ToHex(stagedFiles->files[i].sha1);
            char dir[4];
            strncpy(dir,hash,4);
            dir[3] = '\0';
            printf("%s\n",dir);
            char dirPath[256];
            snprintf(dirPath,sizeof(dirPath),"./.bolt/obj/%s",dir);
            struct stat st = {0};
        if (stat("./.bolt/obj", &st) == -1) {
            if (mkdir("./.bolt/obj") == -1) {
                perror("Error creating ./bolt/obj directory");
                continue;
            }
        }
        
        // Create the directory
        if (mkdir(dirPath) == -1) {
            perror("Error creating directory");
        }

        }else if(stagedFiles->files[i].type == FILE_TYPE_DIR){
            // for directory
        }
    }
}