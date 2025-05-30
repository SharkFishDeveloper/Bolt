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
#include "checkHead.h"
#include "showLogs.h"
#include "createNewBranch.h"
#include "gotToBranch.h"
#include "readLastCommit.h"
#include "fileDirFunctions.h"     


void raw_decompress_index(const char *index_path) {
    FILE *file = fopen(index_path, "rb");
    if (!file) {
        perror("Error opening index file");
        return;
    }

    int file_count;
    fread(&file_count, sizeof(int), 1, file);
    printf("%d\n", file_count);  // Just print number of files

    for (int i = 0; i < file_count; i++) {
        int path_len;
        fread(&path_len, sizeof(int), 1, file);
        printf("%d\n", path_len);

        char *path = (char *)malloc(path_len + 1);
        fread(path, sizeof(char), path_len, file);
        path[path_len] = '\0';
        printf("%s\n", path);

        unsigned char sha1[40];
        fread(sha1, sizeof(unsigned char), 40, file);
        for (int j = 0; j < 40; j++) {
            printf("%02x", sha1[j]);
        }
        printf("\n");

        int type;
        fread(&type, sizeof(int), 1, file);
        printf("%d\n", type);

        int mode;
        fread(&mode, sizeof(int), 1, file);
        printf("%d\n", mode);

        free(path);
    }

    fclose(file);
}

int main(int argc,char* argv[]){
    if(argc == 2){
        if(strcmp(argv[1],"init")==0){
            init();
        }
        else if(strcmp(argv[1],"add") == 0){
            F_STRUCT_ARRAY data = stageDirFiles(".",NULL,0);
            stage(&data);
        }
        else if(strcmp(argv[1],"status")==0){
            STAGE_FILE_STRUCT result = status();
            printStagingResult(result);
        }
        else if(strcmp(argv[1],"logs")==0){
            showLogs();
        }
    }
    else if(argc == 3){
        if(strcmp(argv[1],"checkout")==0){
            char *commitId = argv[2];
            gotoPreviousCommitId(commitId,0);  // Call the function
        }
        else if(strcmp(argv[1],"goto")==0){
            char *branchName = argv[2];
            gotToBranch(branchName);
        }
    }
    else if(argc == 4){
        if(strcmp(argv[1],"commit")==0 && strcmp(argv[2], "-m") == 0){
            char* message = argv[3];
            F_STRUCT_ARRAY stagedFiles = read_index(".bolt/index.bin");
            commit(&stagedFiles,message);
        }
        else if(strcmp(argv[1],"checkout")==0 && strcmp(argv[2], "-b") == 0){
            char* commitId = argv[3];
            createNewBranch(commitId);
        }
    }else{
        printf("No command found.\n");
        printf("Available commands:\n");
        printf("  init\n");
        printf("  add\n");
        printf("  status\n");
        printf("  logs\n");
        printf("  checkout <commit_id>\n");
        printf("  goto <branch_name>\n");
        printf("  commit -m <message>\n");
        printf("  checkout -b <commit_id>\n");
    }
}





// else{
    // makeRecursivePath("c1/c2/c3/c4/");
    // removeFileAndDeleteEmptyDirs("c1/c2/c3/a.txt");
    // removeEmptyDirsUpward("c1/c2/c3/");

    // readLastCommit("main");
// }  










// else{
//     char *name = findSHA1("./package.json");
//     char *hash = sha1ToHex(name);
//     printf("hash -> %s \n",hash);
// }