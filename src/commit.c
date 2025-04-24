#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include "commit.h"                
#include "file_struct.h"
#include "sha1ToHex.h"

void commit(F_STRUCT_ARRAY *stagedFiles,char* message){
    for(int i = 0 ;i<stagedFiles->count;i++){
        //check if index and previos commit blob files are same, to avoid duplication
        if(stagedFiles->files[i].type == FILE_TYPE_FILE){
            printf("h\n");
            char *hash = sha1ToHex(stagedFiles->files[i].sha1);
            char dir[4];
            strncpy(dir,hash,4);
            dir[3] = '\0';
            char dirPath[256];
            snprintf(dirPath,sizeof(dirPath),"./bolt/obj/%s",dir);
            mkdir(dirPath);

        }else if(stagedFiles->files[i].type == FILE_TYPE_DIR){
            // for directory
        }
    }
}