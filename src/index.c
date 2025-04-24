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
#include "printStagingResult.h"

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
            STAGE_FILE_STRUCT result = status();
            printStagingResult(result);
        }
    }
}