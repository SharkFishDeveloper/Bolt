#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>  
#include "commit.h"                
#include "file_struct.h"
#include "sha1ToHex.h"
#include "lz4.h"
#include "myhashmap.h"


void createBlobObjects(F_STRUCT *file, const char *dirPath, const char *path, HashMap *map);
int checkBoltIgnore();

void commit(F_STRUCT_ARRAY *stagedFiles,char* message){
    if(stagedFiles->count == 0){
        printf("\x1b[33mPlease add first using - <bolt add>\x1b[0m\n");
        return;
    }
    int isCheckDir = checkBoltIgnore();
    HashMap map;
    initHashMap(&map, 100);
    for(int i = 0 ;i<stagedFiles->count;i++){
        F_STRUCT *arr = &stagedFiles->files[i];
        if (arr->type == FILE_TYPE_FILE){
            const char *hash = sha1ToHex(arr->sha1);
            char dir[4] = { hash[0], hash[1], hash[2], '\0' };
            const char *path = hash + 3;
            char dirPath[256];
            snprintf(dirPath, sizeof(dirPath), "./.bolt/obj/%s", dir);
            createBlobObjects(arr, dirPath, path, &map);
        }
        else if (arr->type == FILE_TYPE_DIR && isCheckDir== 0){
            
        }
        // long fsize, csize;
        // if (getHashMap(&map, arr->file, &fsize, &csize)) {
        //     printf("VALUE OF: %s => %ld / %ld\n",arr->file, fsize, csize);
        // }
    }   
    // for(int i = 0 ;i<stagedFiles->count;i++){
    //     F_STRUCT *arr = &stagedFiles->files[i];
    //     long fsize, csize;
    //     if (getHashMap(&map, arr->file, &fsize, &csize)) {
    //         printf("VALUE OF: %s => %ld / %ld\n",stagedFiles[i].files, fsize, csize);
    //     }
    // }
    // printf("-----");
    freeHashMap(&map);
}

void createBlobObjects(F_STRUCT *file, const char *dirPath, const char *path, HashMap *map) {
    FILE *fp = fopen(file->file, "rb");
    
    if (!fp) {
        perror("File open failed");
        return;
    }

    fseek(fp, 0, SEEK_END);
    long inputSize = ftell(fp);
    rewind(fp);

    if (inputSize <= 0) {
        fclose(fp);
        return;
    }

    char *src = malloc(inputSize);
    fread(src, 1, inputSize, fp);
    fclose(fp);

    int maxCompressedSize = LZ4_compressBound(inputSize);
    char *compressed = malloc(maxCompressedSize);
    int compressedSize = LZ4_compress_default(src, compressed, inputSize, maxCompressedSize);

    if (compressedSize <= 0) {
        fprintf(stderr, "Compression failed for %s\n", file->file);
        goto cleanup;
    }

    // ✅ Save data to hashmap here
    if (map) {
        setHashMap(map, file->file, inputSize, compressedSize);
    }

    mkdir(dirPath);  

    char fullPath[512];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", dirPath, path);
    if (access(fullPath, F_OK) == 0) {
        goto cleanup;
    }

    FILE *out = fopen(fullPath, "wb");
    if (!out) {
        perror("Write failed");
        goto cleanup;
    }
    fwrite(compressed, 1, compressedSize, out);
    fclose(out);

cleanup:
    free(src);
    free(compressed);
}

int checkBoltIgnore(){
    FILE *boltkeep = fopen("./.bolt/.boltignore","r");
    char buffer[6];
    size_t bytesRead = fread(buffer,sizeof(char),5,boltkeep);
    buffer[bytesRead] = '\0';
    fclose(boltkeep);
    return (bytesRead == 5 && strcmp(buffer, "false") == 0);
}