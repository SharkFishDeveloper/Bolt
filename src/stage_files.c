#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "stage_files.h"                

// FUNCTION PROTOTYPE (Start)
void stage(char *basepath,int *fileCount);
void list_files(char *basepath,char ***file_array,int *fileCount);
int is_ignored(char *file_name);
void load_ignore_list();
int is_directory_empty(const char *path);
// (End)

#define MAX_IGNORE_ENTRIES 200
char *ignore_list[MAX_IGNORE_ENTRIES];
int ignore_count = 0; 

void stage(char *basepath,int *fileCount){
    char **file_array = NULL;
    load_ignore_list();
    list_files(basepath,&file_array, fileCount);
    for(int i =0;i< *fileCount;i++){
        printf("File: %s\n", file_array[i]);
        free(file_array[i]);
    }
    printf("FileCount: %d\n", *fileCount);
    free(file_array);
}

void list_files(char *basepath,char ***file_array,int *fileCount){
    struct dirent *entry; 
    DIR *dp = opendir(basepath);
    if(dp ==NULL){
        perror("openedir failed");
        return;
    }
    
    while ((entry = readdir(dp)) != NULL){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        if(is_ignored(entry->d_name) == 1)continue;
        char full_path[1024];
        snprintf(full_path,sizeof(full_path),"%s/%s",basepath,entry->d_name);
        struct stat statbuf;
        if(stat(full_path,&statbuf) == 0 && S_ISDIR(statbuf.st_mode)){
            if (is_directory_empty(full_path)) {
                *file_array = realloc(*file_array, sizeof(char*) * (*fileCount + 1));
                (*file_array)[*fileCount] = strdup(full_path);
                (*fileCount)++;
            }
            list_files(full_path, file_array, fileCount);
        }else{
            *file_array = realloc(*file_array,sizeof(char*)*(*fileCount+1));
            (*file_array)[*fileCount] = strdup(full_path);
            (*fileCount)++; 
        }
    }
    closedir(dp); 
}

void load_ignore_list() {
    FILE *ignore_file = fopen(".boltignore","r");
    if (ignore_file == NULL) {
        printf("No .boltinore found");
        return; // No .boltignore file found, nothing to ignore
    }
    char line[1024];
    while(fgets(line,sizeof(line),ignore_file) != NULL){
        line[strcspn(line,"\n")]=0;
        if(ignore_count > MAX_IGNORE_ENTRIES){
            printf("Max size of .boltignore reached, Some entries will be dicarded");
            return;
        }else{
            ignore_list[ignore_count] = strdup(line);
            ignore_count++;
        }
    }
    fclose(ignore_file);
}

int is_ignored(char *file_name){
    for(int i = 0; i < ignore_count;i++){
        if(strcmp(file_name,ignore_list[i])==0){
            return 1;
        }
    }
    return 0;
}


int is_directory_empty(const char *path) {
    DIR *dir = opendir(path);
    if (dir == NULL) return 0; // Treat as not empty or inaccessible

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            closedir(dir);
            return 0; // Found a real entry
        }
    }
    closedir(dir);
    return 1; // Nothing but "." and ".."
}