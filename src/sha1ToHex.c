#include <stdlib.h>
#include <stdio.h>
#include "sha1ToHex.h"                
                
char* sha1ToHex(const unsigned char* sha1) {
    int len = 20;
    char* hex = malloc(20 * 2 + 1); // 2 chars per byte + null terminator
    for (int i = 0; i < len; i++) {
        sprintf(hex + i * 2, "%02x", sha1[i]);
    }
    hex[len * 2] = '\0'; // null terminate
    return hex;
}