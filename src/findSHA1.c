#include "sha1.h"
#include "findSHA1.h"                
#include <stdlib.h>               
#include <stdio.h>

#define SHA1_DIGEST_LENGTH 20

char *findSHA1(const char *filename) {
    FILE *file = fopen(filename, "rb");
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

    return hexstr;
}