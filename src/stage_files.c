#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include "stage_files.h"
#include "file_struct.h"
#include "findSHA1.h"
#include "sha1ToHex.h"
#include "ht.h"

#define MAX_IGNORE_ENTRIES 400
#define INITIAL_CAPACITY 200

char *ignore_list[MAX_IGNORE_ENTRIES];
int ignore_count = 0;

F_STRUCT_ARRAY stageDirFiles(char *basepath,ht *map,int i);
void list_files(char *basepath, F_STRUCT_ARRAY *file_array,ht *map,int i);
int is_ignored(const char *file_name);
void load_ignore_list();
int is_directory_empty(const char *path);

//-> Start
F_STRUCT_ARRAY stageDirFiles(char *basepath,ht *map,int i) {
    
    F_STRUCT_ARRAY file_array;
    file_array.count = 0;
    file_array.capacity = INITIAL_CAPACITY;
    file_array.files = malloc(file_array.capacity * sizeof(F_STRUCT));
    if (!file_array.files) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    load_ignore_list();
    list_files(basepath, &file_array,map,i);
    return file_array;
}

void list_files(char *basepath, F_STRUCT_ARRAY *file_array, ht *map,int i) {
    DIR *dp = opendir(basepath);
    if (!dp) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        if (is_ignored(entry->d_name)) continue;

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", basepath, entry->d_name);

        struct stat statbuf;
        if (stat(full_path, &statbuf) != 0) continue;

        if (S_ISDIR(statbuf.st_mode)) {
            if(i==1){ // <- i is used only for gotToPreviousCommitId, else i should be 0
                // printf("I run for -> %s ",full_path);
                if (is_directory_empty(full_path)) {
                    if (file_array->count >= file_array->capacity) {
                        file_array->capacity *= 2;
                        file_array->files = realloc(file_array->files, file_array->capacity * sizeof(F_STRUCT));
                        if (!file_array->files) {
                            perror("realloc failed");
                            exit(EXIT_FAILURE);
                        }
                    }
                    file_array->files[file_array->count].file = strdup(full_path);
                    file_array->files[file_array->count].type = FILE_TYPE_DIR;
                    file_array->files[file_array->count].sha1 = "dummy";
                    file_array->files[file_array->count].mode = 10677;
                    if(map){
                        ht_set(map, full_path, (void*)"dummy");
                    }
                    file_array->count++;
                }
            }else if(i == 0){
                if (is_directory_empty(full_path)) {
                    continue; // <- skip empty directory
                }
            }
            list_files(full_path, file_array,map,i); //! <- where to put this line [BUG: it can be bug] 
        } else {
            // printf("/////////////////////");
            if (file_array->count >= file_array->capacity) {
                file_array->capacity *= 2;
                file_array->files = realloc(file_array->files, file_array->capacity * sizeof(F_STRUCT));
                if (!file_array->files) {
                    perror("realloc failed");
                    exit(EXIT_FAILURE);
                }
            }
            file_array->files[file_array->count].file = strdup(full_path);
            file_array->files[file_array->count].type = FILE_TYPE_FILE;
            file_array->files[file_array->count].mode = 10677;

            char *temp = findSHA1(full_path); 
            char *k = sha1ToHex(temp); // can be issue ?
            file_array->files[file_array->count].sha1 = k;
            if (map != NULL){
                ht_set(map, full_path, (void*)k);
            }
            file_array->count++;
        }
    }
    closedir(dp);
}

void load_ignore_list() {
    FILE *ignore_file = fopen(".boltignore", "r");
    if (!ignore_file) {
        printf("No .boltignore found\n");
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), ignore_file) != NULL) {
        line[strcspn(line, "\n")] = 0; // Trim newline
        if (ignore_count >= MAX_IGNORE_ENTRIES) {
            printf("Max size of .boltignore reached. Some entries will be discarded.\n");
            break;
        }
        ignore_list[ignore_count++] = strdup(line);
    }

    fclose(ignore_file);
}

int is_ignored(const char *file_name) {
    if (ignore_list[0] == NULL) {
        load_ignore_list();
    }
    for (int i = 0; i < ignore_count; i++) {
        if (strcmp(file_name, ignore_list[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_directory_empty(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) return 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            closedir(dir);
            return 0;
        }
    }
    closedir(dir);
    return 1;
}



// if (is_directory_empty(full_path)) {
            //     if (file_array->count >= file_array->capacity) {
            //         file_array->capacity *= 2;
            //         file_array->files = realloc(file_array->files, file_array->capacity * sizeof(F_STRUCT));
            //         if (!file_array->files) {
            //             perror("realloc failed");
            //             exit(EXIT_FAILURE);
            //         }
            //     }
            //     file_array->files[file_array->count].file = strdup(full_path);
            //     file_array->files[file_array->count].type = FILE_TYPE_DIR;
            //     file_array->files[file_array->count].sha1 = "dummy";
            //     file_array->files[file_array->count].mode = 10677;
            //     if(map){
            //         ht_set(map, full_path, (void*)"dummy");
            //     }
            //     file_array->count++;
            // }