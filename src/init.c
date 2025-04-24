#include <dirent.h>
#include <stdio.h>               
#include <stdlib.h>
#include <sys/stat.h>  

#include "init.h"                

void init(){
    char *directory = ".bolt";
    struct stat fs;
    if(stat(directory,&fs)==0){
        printf(".bolt is already initialised");
    }
    else if (mkdir(directory) == 0) {
        FILE *f1 = fopen(".bolt/index.bin","w");
        FILE *f2 = fopen(".boltignore","w");
        fclose(f1);
        fclose(f2);
    }
}