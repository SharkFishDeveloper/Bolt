#include <stdio.h>
#include <string.h>
#include <time.h>
#include "stage_files.h"
#include "init.h"

int main(int argc,char* argv[]){
    if(argc == 2){
        if(strcmp(argv[1],"init")==0){
            init();
        }
        else if(strcmp(argv[1],"add") == 0){
            int fileCount = 0;
            stage(".", &fileCount);
        }
    }
}