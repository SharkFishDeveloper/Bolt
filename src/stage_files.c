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

// Globals
char *ignore_list[MAX_IGNORE_ENTRIES];
int ignore_count = 0;

// Function Prototypes
F_STRUCT_ARRAY stageDirFiles(char *basepath,ht *map);
void list_files(char *basepath, F_STRUCT_ARRAY *file_array,ht *map);
int is_ignored(const char *file_name);
void load_ignore_list();
int is_directory_empty(const char *path);

// --------------------------------

F_STRUCT_ARRAY stageDirFiles(char *basepath,ht *map) {
    
    F_STRUCT_ARRAY file_array;
    file_array.count = 0;
    file_array.capacity = INITIAL_CAPACITY;
    file_array.files = malloc(file_array.capacity * sizeof(F_STRUCT));
    if (!file_array.files) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    load_ignore_list();
    list_files(basepath, &file_array,map);
    return file_array;
}

void list_files(char *basepath, F_STRUCT_ARRAY *file_array,ht *map) {
    DIR *dp = opendir(basepath);
    if (!dp) {
        perror("opendir failed");
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
                file_array->files[file_array->count].sha1 = NULL;
                file_array->files[file_array->count].mode = 10677;
                ht_set(map, full_path, (void*)"dummy");
                file_array->count++;
            }
            list_files(full_path, file_array,map); 
        } else {
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
            file_array->files[file_array->count].sha1 = findSHA1(full_path); 
            file_array->files[file_array->count].mode = 10677;
            char *k = sha1ToHex(file_array->files[file_array->count].sha1);
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
