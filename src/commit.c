#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>  
#include <time.h>
#include "commit.h"                
#include "file_struct.h"
#include "sha1ToHex.h"
#include "lz4.h"
#include "myhashmap.h"
#include "hexGenerator.h"


void createBlobObjects(F_STRUCT *file, const char *dirPath, const char *path, HashMap *map);
int checkBoltIgnore();
int createMetaDataCommitFile(F_STRUCT_ARRAY *stagedFiles,HashMap *map,int isCheckDir,char* message);
int readMetaDataCommitFile();

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
    }  
    createMetaDataCommitFile(stagedFiles,&map,isCheckDir,message); 
    readMetaDataCommitFile();
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

int createMetaDataCommitFile(F_STRUCT_ARRAY *stagedFiles,HashMap *map,int isCheckDir,char* message){
    time_t now;
    time(&now);
    char *author = "anonymous@gmail.com";
    char *name = "anonymous_name";

    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));

    char hex[41];
    generateRandomHex40(hex);
    int bufferSize = 1024;
    char *data = malloc(1024); 
    int offset = 0;

    offset += snprintf(data + offset, 1024 - offset, "AUTHOR:<%s>\n", author);  // author email
    offset += snprintf(data + offset, 1024 - offset, "NAME:<%s>\n", name);     // author name
    offset += snprintf(data + offset, 1024 - offset, "TIME:<%s>\n", buf);  // timestamp

    long totalDataSize = 0;
    for (int i = 0; i < stagedFiles->count; i++) {
        F_STRUCT *arr = &stagedFiles->files[i];
        if (offset >= bufferSize - 100) {
            bufferSize *= 2;
            data = realloc(data, bufferSize);
        }
        if (arr->type == FILE_TYPE_FILE) { 
            long fsize, csize;
            getHashMap(map,arr->file,&fsize,&csize);
            printf("=> %s , %d ,%d\n",arr->file,fsize,csize);
            totalDataSize += fsize;
            offset += snprintf(data + offset, 1024 - offset, "%s|%s|%s|%d|%d\n",arr->file, "File", sha1ToHex(arr->sha1), fsize, csize);
        }
        else if(arr->type == FILE_TYPE_DIR){
            offset += snprintf(data + offset, bufferSize - offset, "%s|%s|%s|%d|%d\n",arr->file,"Dir","NULL", 0, 0);
        }
    }
    // offset += snprintf(data + offset, 1024 - offset, "SIZE:<%d>",totalDataSize);
    
    int maxCompressedSize = LZ4_compressBound(offset);  // Calculate the max compressed size
    char *compressedData = malloc(maxCompressedSize);
    int compressedSize = LZ4_compress_default(data, compressedData, offset, maxCompressedSize);

    if (compressedSize <= 0) {
        fprintf(stderr, "Compression failed\n");
        free(data);
        free(compressedData);
        return -1;
    }

    FILE *out = fopen("./.bolt/commitnew", "wb");
    if (!out) {
        perror("Write failed");
        free(data);
        free(compressedData);
        return -1;
    }

    fwrite(&compressedSize, sizeof(int), 1, out); // Write compressed size
    fwrite(compressedData, 1, compressedSize, out); // Write compressed data
    fclose(out);

    free(data);
    free(compressedData);
}


int readMetaDataCommitFile() {
    // Open the commit file
    FILE *in = fopen("./.bolt/commitnew", "rb");
    if (!in) {
        perror("Failed to open commit file");
        return -1;
    }

    // Step 1: Read the compressed size
    int compressedSize = 0;
    fread(&compressedSize, sizeof(int), 1, in);
    if (compressedSize <= 0) {
        fprintf(stderr, "Invalid compressed size\n");
        fclose(in);
        return -1;
    }

    // Step 2: Allocate memory for the compressed data
    char *compressedData = malloc(compressedSize);
    if (!compressedData) {
        perror("Memory allocation failed for compressed data");
        fclose(in);
        return -1;
    }

    // Step 3: Read the compressed data
    fread(compressedData, 1, compressedSize, in);
    fclose(in);  // Close the file after reading

    // Step 4: Decompress the data
    int decompressedSize = 1024 * 10;  // Start with an arbitrary size
    char *decompressedData = malloc(decompressedSize);
    if (!decompressedData) {
        perror("Memory allocation failed for decompressed data");
        free(compressedData);
        return -1;
    }

    int actualDecompressedSize = LZ4_decompress_safe(compressedData, decompressedData, compressedSize, decompressedSize);
    if (actualDecompressedSize < 0) {
        fprintf(stderr, "Decompression failed\n");
        free(compressedData);
        free(decompressedData);
        return -1;
    }

    // Step 5: Now we can parse the decompressed data
    printf("Decompressed data: \n%s\n", decompressedData);  // Example: print the decompressed data

    // Parse the metadata string as needed (e.g., extract file info, author, timestamp, etc.)

    // Cleanup
    free(compressedData);
    free(decompressedData);
    return 0;
}