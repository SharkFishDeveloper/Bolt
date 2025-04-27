#include "sha1.h"
#include "findSHA1.h"                
#include <stdlib.h>               
#include <stdio.h>

#define SHA1_DIGEST_LENGTH 20

char *findSHA1(const char *filename) {
    FILE *file = fopen(filename, "rb");
    // printf("Full path in sha1 %s\n",filename);
    if (!file) return NULL;

    SHA1_CTX ctx;
    SHA1Init(&ctx);

    unsigned char buffer[1024];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        SHA1Update(&ctx, buffer, bytesRead);
    }

    fclose(file);

    unsigned char hash[SHA1_DIGEST_LENGTH];
    SHA1Final(hash, &ctx);

    // Convert to hex string
    char *hexstr = malloc(SHA1_DIGEST_LENGTH * 2 + 1);
    for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
        sprintf(hexstr + (i * 2), "%02x", hash[i]);
    }
    hexstr[SHA1_DIGEST_LENGTH * 2] = '\0';

    char *normal_str = malloc(SHA1_DIGEST_LENGTH + 1);  // One byte for each hex byte
    for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
        normal_str[i] = (char) hash[i];  // Convert each byte to a char
    }
    normal_str[SHA1_DIGEST_LENGTH] = '\0';  // Null terminate the string

    // Free the hex string memory as it's no longer needed
    free(hexstr);
    // printf("SH1 %s",nomakrmal_str);
    return normal_str;

    // return hexstr;
}