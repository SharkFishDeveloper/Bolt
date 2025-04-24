#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stage.h"
#include "file_struct.h"

#define SHA1_LENGTH 20  // Correct SHA1 hash length (20 bytes)

void stage(F_STRUCT_ARRAY *file_array) {
    FILE *index_file = fopen(".bolt/index.bin", "wb");
    if (!index_file) {
        perror("Error opening .bolt/index.bin");
        return;
    }

    // First write the number of files
    fwrite(&file_array->count, sizeof(int), 1, index_file);

    for (int i = 0; i < file_array->count; i++) {
        F_STRUCT *f = &file_array->files[i];

        // Write file path length
        int path_len = strlen(f->file);
        fwrite(&path_len, sizeof(int), 1, index_file);

        // Write actual file path string (no null terminator)
        fwrite(f->file, sizeof(char), path_len, index_file);

        // Write raw SHA1 hash (20 bytes)
        if (f->sha1 != NULL) {
            fwrite(f->sha1, sizeof(unsigned char), SHA1_LENGTH, index_file);
        } else {
            unsigned char null_sha1[SHA1_LENGTH] = {0};  // 20-byte zeroed hash
            fwrite(null_sha1, sizeof(unsigned char), SHA1_LENGTH, index_file);
        }

        // Write file type (enum as int)
        fwrite(&f->type, sizeof(int), 1, index_file);

        // Write file mode
        fwrite(&f->mode, sizeof(int), 1, index_file);
    }

    fclose(index_file);
}

F_STRUCT_ARRAY read_index(const char *path) {
    // printf("HERE");
    F_STRUCT_ARRAY result;
    result.count = 0;
    result.files = NULL;


    FILE *f = fopen(path, "rb");

    if (fseek(f, 0, SEEK_END) != 0) {
        // perror("fseek failed");
        fclose(f);
        exit(EXIT_FAILURE);
    }

    long size = ftell(f);
    if (size == -1L) {
        // perror("ftell failed");
        fclose(f);
        exit(EXIT_FAILURE);
    }

    rewind(f);

    // If file is empty, return empty result
    if (size == 0) {
        fclose(f);
        // printf("INDEX IS EMPTY\n");
        return result;
    }

    fread(&result.count, sizeof(int), 1, f);
    result.files = malloc(result.count * sizeof(F_STRUCT));

    for (int i = 0; i < result.count; i++) {
        F_STRUCT *entry = &result.files[i];

        // Read path length
        int path_len;
        fread(&path_len, sizeof(int), 1, f);

        // Read path
        entry->file = malloc(path_len + 1);
        fread(entry->file, sizeof(char), path_len, f);
        entry->file[path_len] = '\0';

        // Read SHA1 (20 bytes)
        entry->sha1 = malloc(SHA1_LENGTH);
        fread(entry->sha1, sizeof(unsigned char), SHA1_LENGTH, f);

        // Read type and mode
        fread(&entry->type, sizeof(int), 1, f);
        fread(&entry->mode, sizeof(int), 1, f);
    }

    fclose(f);
    return result;
}
