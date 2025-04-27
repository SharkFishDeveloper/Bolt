#include <stdlib.h>
#include <stdio.h>
#include "lz4.h"
#include "decompressLz4.h"                
                
char *decompressFile(const char *filepath, int *out_size) {
    FILE *in = fopen(filepath, "rb");
    if (!in) {
        perror("Failed to open file");
        return NULL;
    }

    // 🛠 Directly get file size
    fseek(in, 0, SEEK_END);
    long compressedSize = ftell(in);
    rewind(in);

    if (compressedSize <= 0) {
        fclose(in);
        return NULL;
    }

    char *compressedData = malloc(compressedSize);
    if (!compressedData) {
        perror("Memory allocation failed for compressed data");
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

    int decompressedSize = compressedSize * 2;
    char *decompressedData = malloc(decompressedSize);
    if (!decompressedData) {
        perror("Memory allocation failed for decompressed data");
        free(compressedData);
        return NULL;
    }

    int actualSize = LZ4_decompress_safe(compressedData, decompressedData, compressedSize, decompressedSize);

    while (actualSize < 0) {
        decompressedSize *= 2;
        char *newBuffer = realloc(decompressedData, decompressedSize);
        if (!newBuffer) {
            perror("Memory reallocation failed");
            free(compressedData);
            free(decompressedData);
            return NULL;
        }
        decompressedData = newBuffer;
        actualSize = LZ4_decompress_safe(compressedData, decompressedData, compressedSize, decompressedSize);
    }

    free(compressedData);

    if (out_size) {
        *out_size = actualSize;
    }

    return decompressedData;
}