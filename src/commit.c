#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>  
#include <time.h>
#include <sys/stat.h> 
#include "commit.h"                
#include "file_struct.h"
#include "sha1ToHex.h"
#include "lz4.h"
#include "myhashmap.h"
#include "hexGenerator.h"
#include "generateSHA1.h"
#include "checkHead.h"

void createBlobObjects(F_STRUCT *file, const char *dirPath, const char *path, HashMap *map);
int checkBoltIgnore();
int createMetaDataCommitFile(F_STRUCT_ARRAY *stagedFiles,HashMap *map,int isCheckDir,char* message);
int   readMetaDataCommitFile();
char* extractParentCommitId(const char *filePath);


void commit(F_STRUCT_ARRAY *stagedFiles,char* message){
    int checkIfOnlastCommit = checkIfCommitExistsInBranch();
    if(stagedFiles->count == 0){
        printf("\x1b[33mPlease add first using - <bolt add>\x1b[0m\n");
        return;
    }
    if(checkIfOnlastCommit == 0){
        printf("\033[1;31mMake a new branch to commit\033[0m\n");
        return;
    }
    int isCheckDir = checkBoltIgnore();
    HashMap map;
    initHashMap(&map, 1000);
    for(int i = 0 ;i<stagedFiles->count;i++){
        F_STRUCT *arr = &stagedFiles->files[i];
        if (arr->type == FILE_TYPE_FILE){
            const char *hash = arr->sha1;
            char dir[4] = { hash[0], hash[1], hash[2], '\0' };
            const char *path = hash + 3;
            char dirPath[256];
            snprintf(dirPath, sizeof(dirPath), "./.bolt/obj/%s", dir);
            createBlobObjects(arr, dirPath, path, &map);
        }
    }  
    int created = createMetaDataCommitFile(stagedFiles,&map,isCheckDir,message); 
    if (created == 1) {
        printf("\033[1;32mCommit Successful\033[0m\n");
    }
    freeHashMap(&map);
}


void createBlobObjects(F_STRUCT *file, const char *dirPath, const char *path, HashMap *map) {
    char pathCheck[100];
    snprintf(pathCheck, sizeof(pathCheck), "./.bolt/obj/%.3s/%.37s", file->sha1, file->sha1 + 3);
    struct stat st;
    stat(file->file, &st);
    long fileSize = st.st_size;

    if(access(pathCheck,F_OK)==0){
        setHashMap(map, file->file, fileSize, 0);
        return;
    }
    else{    
        // fix this , it checks size two times
        char *src = malloc(fileSize);   
        // printf("size %d->",fileSize); 
        FILE *fp = fopen(file->file, "r");
        size_t bytesRead = fread(src, 1, fileSize, fp);
        if (bytesRead != fileSize) {
            // fprintf(stderr, "Warning: Only read %zu of %ld bytes from %s\n", bytesRead, fileSize, file->file);
            fileSize = bytesRead;  // Adjust fileSize to actual read amount
        }
        fread(src, 1, fileSize, fp);
        fclose(fp);
        int maxCompressedSize = LZ4_compressBound(fileSize);
        char *compressed = malloc(maxCompressedSize);
        int compressedSize = LZ4_compress_default(src, compressed, fileSize, maxCompressedSize);
    
        if (compressedSize <= 0) {
            fprintf(stderr, "Compression failed for %s\n", file->file);
            goto cleanup;
        }
        if (map) {
            setHashMap(map, file->file, fileSize, 0);
        }
        
        mkdir(dirPath);  
    
        FILE *out = fopen(pathCheck, "wb");
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
}

int checkBoltIgnore(){
    FILE *boltkeep = fopen("./.bolt/.boltkeep","r");
    char buffer[6];
    size_t bytesRead = fread(buffer,sizeof(char),5,boltkeep);
    buffer[bytesRead] = '\0';
    fclose(boltkeep);
    return (bytesRead == 5 && strcmp(buffer, "false") == 0);
}

int createMetaDataCommitFile(F_STRUCT_ARRAY *stagedFiles,HashMap *map,int isCheckDir,char* message){
    srand(time(NULL));
    FILE *HEAD = fopen("./.bolt/HEAD","r"); // extract <branch> name
    char *commitName =  malloc(60);
    fgets(commitName,256,HEAD);
    char *branchName = strrchr(commitName, ':');
    branchName+=2;
    branchName[strcspn(branchName, "\n")] = '\0';
    
    char fullPathCommitRefs[50];
    snprintf(fullPathCommitRefs,sizeof(fullPathCommitRefs),"./.bolt/%s",branchName);
    
    FILE *refsBranchNamePath = fopen(fullPathCommitRefs,"r");
    char commitId[50] = {0};
    fgets(commitId, sizeof(commitId), refsBranchNamePath);
    commitId[strcspn(commitId, "\n")] = '\0';
    fclose(refsBranchNamePath);
    
    time_t now;
    time(&now);

    char *author_mail = "anonymous@gmail.com";
    char *name   = "anonymous_name";

    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    char hex[41]; // this is for current commitID
    generateRandomHex40(hex);

    int bufferSize = 1024;
    char *data = malloc(1024); 

    int bufferSizeTree = 1024;
    char *treeData = malloc(1024); 
    
    int offset = 0;
    int tree_offset = 0;
    
   
    long totalDataSize = 0;
    int fileCount = 0;
    int compressedfileSize = 0;
    for (int i = 0; i < stagedFiles->count; i++) {
        F_STRUCT *arr = &stagedFiles->files[i];

        if (tree_offset >= bufferSizeTree - 100) {
            bufferSizeTree *= 2;
            treeData = realloc(treeData, bufferSizeTree);
        }
        if (arr->type == FILE_TYPE_FILE) { 
            long fsize = -1, csize = -1;
            getHashMap(map,arr->file,&fsize,&csize);
            totalDataSize += fsize;
            tree_offset += snprintf(treeData + tree_offset, bufferSizeTree - tree_offset, "%s|%s|%s\n",arr->file, "File", arr->sha1);
        }
    }
    char treeHash[41];
    generateSHA1(treeData,treeHash);
   

    char parentTreeDir[4] = { commitId[0], commitId[1], commitId[2], '\0' };
    char parentTreeFile[38];
    strncpy(parentTreeFile, commitId + 3, 37);
    parentTreeFile[37] = '\0';

    char parentCommitIdPath[500];
    snprintf(parentCommitIdPath,sizeof(parentCommitIdPath),"./.bolt/obj/%s/%s",parentTreeDir,parentTreeFile);
  
    snprintf(fullPathCommitRefs,sizeof(fullPathCommitRefs),"./.bolt/%s",branchName);

    if(strlen(commitId) != 0){
        char *parentTreeHash = extractParentCommitId(parentCommitIdPath);
        if(parentTreeHash!=NULL && strcmp(parentTreeHash,treeHash)==0){
            printf("\033[1;33mNo changes found, cannot commit\n\033[0m");
            return -1;
        }
    }
        
    offset += snprintf(data + offset, bufferSize - offset, "TREE:%s\n", treeHash);  // author email
    offset += snprintf(data + offset, bufferSize - offset, "AUTHOR:%s\n", author_mail);  // author email
    offset += snprintf(data + offset, bufferSize - offset, "NAME:%s\n", name);     // author name
    offset += snprintf(data + offset, bufferSize - offset, "TIME:%s\n", buf);  // timestamp
    if (strlen(commitId) > 0) {
        offset += snprintf(data + offset, bufferSize - offset, "PARENT_COMMIT:%s\n", commitId);
    }else{
        offset += snprintf(data + offset, bufferSize - offset, "PARENT_COMMIT:NULL\n");
    }
    offset += snprintf(data + offset, 1024 - offset, "SIZE:%d",totalDataSize);

    int maxCompressedSize = LZ4_compressBound(offset);  // Calculate the max compressed size
    char *compressedData = malloc(maxCompressedSize);
    int compressedSize = LZ4_compress_default(data, compressedData, offset, maxCompressedSize);

    int maxCompressedSizeTree = LZ4_compressBound(tree_offset);  // Calculate the max compressed size
    char *compressedDataTree = malloc(maxCompressedSizeTree);
    int compressedSizeTree = LZ4_compress_default(treeData, compressedDataTree, tree_offset, maxCompressedSizeTree);


    if (compressedSize <= 0 || maxCompressedSizeTree<=0) {
        fprintf(stderr, "Compression failed\n");
        free(data);
        free(compressedData);
        return -1;
    }

    char dir[4] = { hex[0], hex[1], hex[2], '\0' };
    char file[38];
    strncpy(file, hex + 3, 37);
    file[37] = '\0';
    // printf("REFS/HEADS/MAIN -> %s\n",fullPathCommitRefs);
   
    FILE *commitFIle = fopen(fullPathCommitRefs, "w");
    fprintf(commitFIle, "%s", hex); // write tree hash of current commit
    fclose(commitFIle);

    char dirFullPath[256]; 
    snprintf(dirFullPath, sizeof(dirFullPath), "./.bolt/obj/%s", dir); 

    mkdir(dirFullPath);
    
    char fileFullPath[256];
    snprintf(fileFullPath, sizeof(fileFullPath), "%s/%s", dirFullPath, file);
    
    char treeFullPath[256];
    char treeDir[4] = { treeHash[0], treeHash[1], treeHash[2], '\0' };
    char treeFile[38];
    strncpy(treeFile, treeHash + 3, 37);
    treeFile[37] = '\0';
    snprintf(treeFullPath, sizeof(treeFullPath), "./.bolt/obj/%s", treeDir);
    
    mkdir(treeFullPath);

    char treefileFullPath[256];
    snprintf(treefileFullPath, sizeof(treefileFullPath), "%s/%s", treeFullPath, treeFile);


    FILE *out = fopen(fileFullPath, "wb");
    FILE *treeFileOut = fopen(treefileFullPath, "wb");


    if (!out || !treeFile) {
        perror("Write failed");
        free(data);
        free(compressedData);
        return -1;
    }

    fwrite(&compressedSize, sizeof(int), 1, out); // Write compressed size
    fwrite(compressedData, 1, compressedSize, out); // Write compressed data

    fwrite(&compressedSizeTree, sizeof(int), 1, treeFileOut); // Write compressed size
    fwrite(compressedDataTree, 1, compressedSizeTree, treeFileOut); 

    //Write to logs in logs/refs/heads/branchname
    char logsPath[80];
    snprintf(logsPath,sizeof(logsPath),"./.bolt/logs/%s",branchName); 
    FILE *fp = fopen(logsPath, "a");
    fprintf(fp, "commit:%s\n", hex);
    fprintf(fp, "author_mail:%s\n", author_mail);
    fprintf(fp, "author:%s\n", name);
    fprintf(fp, "timestamp:%s\n", buf);
    fprintf(fp, "message:%s\n", message);
    fprintf(fp, "----\n"); 
    fclose(fp);
    //-------------------------------------------

    fclose(out);
    fclose(treeFileOut);

    free(data);
    free(compressedData);
    return 1;
}

int readMetaDataCommitFile() {
    FILE *in = fopen("./.bolt/commitnew", "rb");
    if (!in) {
        perror("Failed to open commit file");
        return -1;
    }
    int compressedSize = 0;
    size_t readSize = fread(&compressedSize, sizeof(int), 1, in);
    if (readSize != 1 || compressedSize <= 0) {
        fprintf(stderr, "Invalid compressed size or failed to read it\n");
        fclose(in);
        return -1;
    }

    // printf("Compressed size: %d\n", compressedSize);

    char *compressedData = malloc(compressedSize);
    if (!compressedData) {
        perror("Memory allocation failed for compressed data");
        fclose(in);
        return -1;
    }

    size_t bytesRead = fread(compressedData, 1, compressedSize, in);
    fclose(in); 

    int decompressedSize = compressedSize * 2;  
    char *decompressedData = malloc(decompressedSize);
    if (!decompressedData) {
        perror("Memory allocation failed for decompressed data");
        free(compressedData);
        return -1;
    }

    int actualDecompressedSize = LZ4_decompress_safe(compressedData, decompressedData, compressedSize, decompressedSize);
    while (actualDecompressedSize < 0) {
        decompressedSize *= 2;
        decompressedData = realloc(decompressedData, decompressedSize);
        if (!decompressedData) {
            perror("Memory reallocation failed for decompressed data");
            free(compressedData);
            return -1;
        }
        actualDecompressedSize = LZ4_decompress_safe(compressedData, decompressedData, compressedSize, decompressedSize);
    }

    free(compressedData);
    free(decompressedData);
    return 0;
}

char* extractParentCommitId(const char *filePath) {
    if (access(filePath, F_OK) != 0) {
        return NULL;
    }
    FILE *in = fopen(filePath, "rb");
    if (!in) {
        perror("Failed to open file");
        return "NULL";
    }

    int compressedSize = 0;
    if (fread(&compressedSize, sizeof(int), 1, in) != 1 || compressedSize <= 0) {
        fprintf(stderr, "Invalid compressed size\n");
        fclose(in);
        return NULL;
    }

    char *compressedData = malloc(compressedSize);
    if (!compressedData) {
        perror("Memory allocation failed (compressedData)");
        fclose(in);
        return NULL;
    }

    if (fread(compressedData, 1, compressedSize, in) != (size_t)compressedSize) {
        fprintf(stderr, "Failed to read compressed data\n");
        free(compressedData);
        fclose(in);
        return NULL;
    }
    fclose(in);

    int decompressedSizeGuess = compressedSize * 4;
    char *decompressedData = malloc(decompressedSizeGuess + 1); // +1 for null terminator
    if (!decompressedData) {
        perror("Memory allocation failed (decompressedData)");
        free(compressedData);
        return NULL;
    }

    int actualDecompressedSize = LZ4_decompress_safe(compressedData, decompressedData, compressedSize, decompressedSizeGuess);
    free(compressedData);

    if (actualDecompressedSize < 0) {
        fprintf(stderr, "Decompression failed\n");
        free(decompressedData);
        return NULL;
    }

    decompressedData[actualDecompressedSize] = '\0'; // Null-terminate

    // 📜 Now find PARENT_COMMIT:
    char *parentCommitLine = strstr(decompressedData, "TREE:");
    if (!parentCommitLine) {
        fprintf(stderr, "TREE not found\n");
        free(decompressedData);
        return NULL;
    }

    parentCommitLine += strlen("TREE:"); // Move after "PARENT_COMMIT:"
    while (*parentCommitLine == ' ') parentCommitLine++; // Skip spaces if any

    // Copy the commit id until newline
    char *newline = strchr(parentCommitLine, '\n');
    if (!newline) {
        newline = parentCommitLine + strlen(parentCommitLine); // End of string
    }

    int commitIdLen = newline - parentCommitLine;
    char *commitId = malloc(commitIdLen + 1); // +1 for null terminator
    if (!commitId) {
        perror("Memory allocation failed (commitId)");
        free(decompressedData);
        return NULL;
    }
    strncpy(commitId, parentCommitLine, commitIdLen);
    commitId[commitIdLen] = '\0'; // Null-terminate

    free(decompressedData);
    return commitId;
}

        // printf("%s TREEDATA\n",treeData);
        // else if(arr->type == FILE_TYPE_DIR && isCheckDir == 0){
            // printf("Dir-> %s",arr->file);
        //     tree_offset += snprintf(treeData + tree_offset, bufferSizeTree - tree_offset, "%s|%s|%s|%d|%d\n",arr->file,"Dir","NULL", 0, 0);
        // }