#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>  
#include "commit.h"                
#include "file_struct.h"
#include "sha1ToHex.h"
#include "lz4.h"

void createBlobObjects(F_STRUCT *file,char *dirPath,char *path);

void commit(F_STRUCT_ARRAY *stagedFiles,char* message){
    if(stagedFiles->count == 0){
        printf("\x1b[33mPlease add first using - <bolt add>\x1b[0m\n");
        return;
    }
    for(int i = 0 ;i<stagedFiles->count;i++){
        F_STRUCT *arr = &stagedFiles->files[i];
        // printf("-> %s \n",arr->file);
        //check if index and previos commit blob files are same, to avoid duplication
        if(arr->type == FILE_TYPE_FILE){
            char *hash = sha1ToHex(arr->sha1);
            char dir[4];
            char path[38];
            strncpy(dir,hash,4);
            strncpy(path, hash + 3, 37);
            dir[3] = '\0';
            path[37] = '\0';
            char dirPath[256];
            snprintf(dirPath,sizeof(dirPath),"./.bolt/obj/%s",dir);
            createBlobObjects(arr,dirPath,path);
            

        }
        else if(stagedFiles->files[i].type == FILE_TYPE_DIR){
            // for directory
        }
    }   
}

void createBlobObjects(F_STRUCT *file,char *dirPath,char *path){
    // mkdir(dirPath);
    FILE *fp = fopen(file->file , "rb");
    fseek(fp, 0, SEEK_END);
    int inputSize = ftell(fp);
    rewind(fp);

    char *src = malloc(inputSize);
    fread(src, 1, inputSize, fp);
    fclose(fp);

    // Compress with LZ4
    int maxCompressedSize = LZ4_compressBound(inputSize);
    char *compressed = malloc(maxCompressedSize);
    int compressedSize = LZ4_compress_default(src, compressed, inputSize, maxCompressedSize);

    if (compressedSize <= 0) {
        fprintf(stderr, "Compression failed\n");
        free(src);
        free(compressed);
        return;
    }

    // ✅ You now have `compressed` with `compressedSize` bytes.
    // Do whatever you want here — like writing to your .vcs/blob dir or storing in memory.
    printf("Compressed %s (%d bytes) -> %d bytes\n", file->file, inputSize, compressedSize);

    // Free memory
    free(src);
    free(compressed);

}

// struct stat st = {0};
// if (stat("./.bolt/obj", &st) == -1) {
//     if (mkdir("./.bolt/obj") == -1) {
//         perror("Error creating ./bolt/obj directory");
//         continue;
//     }
// }

// Create the directory