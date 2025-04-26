
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "generateSHA1.h"    
#define HASH_LENGTH 40

void generateSHA1(char *data, char *output) {
    unsigned long hash = 0;
    int c;

    // Remove randomness and directly hash the data
    while ((c = *data++)) {
        hash = (hash << 5) + hash + c;  
    }

    // Format the hash value into a hex string (40 characters)
    for (int i = 0; i < HASH_LENGTH; i++) {
        snprintf(output + i * 2, 3, "%02lx", (hash >> (i * 4)) & 0xF); 
    }

    output[HASH_LENGTH] = '\0';  // Null terminate the hex string
}