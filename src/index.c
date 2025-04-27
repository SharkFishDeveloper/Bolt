#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "stage_files.h"
#include "init.h"
#include "file_struct.h"
#include "findSHA1.h"
#include "stage.h"
#include "status.h"
#include "stage_file_struct.h"
#include "printStagingResult.h"
#include "sha1ToHex.h"
#include "commit.h"
#include "gotoPrevCommitId.h"

int main(int argc,char* argv[]){
    if(argc == 2){
        if(strcmp(argv[1],"init")==0){
            init();
        }
        else if(strcmp(argv[1],"add") == 0){
            F_STRUCT_ARRAY data = stageDirFiles(".",NULL);
            stage(&data);
        }
        else if(strcmp(argv[1],"status")==0){
            STAGE_FILE_STRUCT result = status();
            printStagingResult(result);
        }
    }
    else if(argc == 3){
        if(strcmp(argv[1],"checkout")==0){
            char *commitId = argv[2];
        
            // Start the timer for all loops
            // clock_t start_time = clock();  
            
            // Loop 10 times to measure time
            // for (int i = 0; i < 50; i++) {
                gotoPreviousCommitId(commitId);  // Call the function
            // }
            
            // End the timer after all loops
            // clock_t end_time = clock();
            
            // Calculate total time taken for 10 loops
            // double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;  
            
            // Print total time taken for all loops
            // printf("\nTotal time taken for 10 loops: %f seconds\n", time_taken);
        }
    }
    else if(argc == 4){
        if(strcmp(argv[1],"commit")==0 && strcmp(argv[2], "-m") == 0){
            char* message = argv[3];
            F_STRUCT_ARRAY stagedFiles = read_index(".bolt/index.bin");
            commit(&stagedFiles,message);
        }
        else if(strcmp(argv[1],"checkout")==0 && strcmp(argv[2], "-b") == 0){
            // char* commitId = argv[3];
            // createNewBranch();

        }
    }    
}












// else{
//     char *name = findSHA1("./package.json");
//     char *hash = sha1ToHex(name);
//     printf("hash -> %s \n",hash);
// }