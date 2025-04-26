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
        mkdir(".bolt/obj");
        mkdir(".bolt/refs");
        mkdir(".bolt/refs/heads");
        FILE *f1 = fopen(".bolt/index.bin","wb");
        FILE *f2 = fopen(".boltignore","w");
        FILE *f3 = fopen(".bolt/.boltkeep","w");
        FILE *f4 = fopen(".bolt/HEAD","w");
        FILE *f5 = fopen(".bolt/refs/heads/main","w");
        fwrite("false",sizeof(char),5,f3);
        fwrite("ref: refs/heads/main",sizeof(char),20,f4);
        // fwrite("ROOT",sizeof(char),4,f5);

        fclose(f1);
        fclose(f2);
        fclose(f3);
        fclose(f4);
        fclose(f5);
    }
}