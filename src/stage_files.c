#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include "stage_files.h"
#include "file_struct.h"

#define MAX_IGNORE_ENTRIES 200
#define INITIAL_CAPACITY 200

// Globals
char *ignore_list[MAX_IGNORE_ENTRIES];
int ignore_count = 0;

// Function Prototypes
void stage(char *basepath, int *fileCount);
void list_files(char *basepath, char ***file_array, int *fileCount, int *capacity);
int  is_ignored(const char *file_name);
void load_ignore_list();
int  is_directory_empty(const char *path);
// --------------------------------

void stage(char *basepath, int *fileCount) {
    int capacity = INITIAL_CAPACITY;
    char **file_array = malloc(sizeof(char *) * capacity);

    F_STRUCT_ARRAY fileStruct;
    fileStruct.

    if (!file_array) {
        perror("malloc failed");
        return;
    }

    load_ignore_list();
    list_files(basepath, &file_array, fileCount, &capacity);
    // calculateSHA1();
    // for(int i = 0 ; i < *fileCount;i++){
    //     printf("%s \n",file_array[i]);
    // }
}

void list_files(char *basepath, char ***file_array, int *fileCount, int *capacity) {
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
                if (*fileCount >= *capacity) {
                    int new_capacity = *capacity * 2;
                    char **new_array = realloc(*file_array, sizeof(char *) * new_capacity);
                    if (!new_array) {
                        perror("realloc failed");
                        exit(EXIT_FAILURE);
                    }
                    *file_array = new_array;
                    *capacity = new_capacity;
                }
                (*file_array)[(*fileCount)++] = strdup(full_path);
                printf("EMPTY DIR - > %s\n",full_path);
            }

            list_files(full_path, file_array, fileCount, capacity);
        } else {
            if (*fileCount >= *capacity) {
                int new_capacity = *capacity * 2;
                char **new_array = realloc(*file_array, sizeof(char *) * new_capacity);
                if (!new_array) {
                    perror("realloc failed");
                    exit(EXIT_FAILURE);
                }
                *file_array = new_array;
                *capacity = new_capacity;
            }
            printf("Reg file - > %s\n",full_path);
            (*file_array)[(*fileCount)++] = strdup(full_path);
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
    if(ignore_list[0] == NULL){
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
