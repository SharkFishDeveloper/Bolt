#include <stdio.h>
#include <string.h>
#include <time.h>
#include "stage_files.h"
#include "init.h"
#include "file_struct.h"
#include "findSHA1.h"
#include "stage.h"
#include "status.h"
#include "stage_file_struct.h"

int main(int argc,char* argv[]){
    if(argc == 2){
        if(strcmp(argv[1],"init")==0){
            init();
        }
        else if(strcmp(argv[1],"add") == 0){
            F_STRUCT_ARRAY data = stageDirFiles(".");
            stage(&data);
        }
        else if(strcmp(argv[1],"status")==0){
            // F_STRUCT_ARRAY data = stageDirFiles(".");
            // for(int i = 0; i < data.count; i++){
            //     F_STRUCT current = data.files[i];
            //     printf("%s\n",current.file);
            // }
            STAGE_FILE_STRUCT result = status();

    // printf("Changes to be committed:\n");

    // if (result.addedFileCount > 0) {
    //     printf("  Added files:\n");
    //     for (int i = 0; i < result.addedFileCount; i++) {
    //         printf("    + %s\n", result.addedFiles[i]);
    //     }
    // }

    // if (result.modedFileCount > 0) {
    //     printf("  Modified files:\n");
    //     for (int i = 0; i < result.modedFileCount; i++) {
    //         printf("    ~ %s\n", result.modedFiles[i]);
    //     }
    // }

    // if (result.deletedFileCount > 0) {
    //     printf("  Deleted files:\n");
    //     for (int i = 0; i < result.deletedFileCount; i++) {
    //         printf("    - %s\n", result.deletedFiles[i]);
    //     }
    // }

    // if (result.addedFileCount == 0 && result.modedFileCount == 0 && result.deletedFileCount == 0) {
    //     printf("  No changes detected.\n");
    // }
        }
    }
}