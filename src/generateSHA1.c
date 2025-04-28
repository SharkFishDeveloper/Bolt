#include <stdio.h>
#include <string.h>
#include <stdint.h>

// SHA-1 constants
#define SHA1_BLOCK_SIZE 64
#define SHA1_DIGEST_SIZE 20

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    uint8_t buffer[SHA1_BLOCK_SIZE];
} SHA1_CTX;

void SHA1_Init(SHA1_CTX* context) {
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}

void SHA1_Transform(SHA1_CTX* context, const uint8_t buffer[SHA1_BLOCK_SIZE]) {
    uint32_t w[80];
    uint32_t a, b, c, d, e;
    uint8_t temp_buffer[SHA1_BLOCK_SIZE];
    memcpy(temp_buffer, buffer, SHA1_BLOCK_SIZE);
    
    for (int i = 0; i < 16; i++) {
        w[i] = (temp_buffer[i * 4] << 24) |
               (temp_buffer[i * 4 + 1] << 16) |
               (temp_buffer[i * 4 + 2] << 8) |
               (temp_buffer[i * 4 + 3]);
    }
    for (int i = 16; i < 80; i++) {
        w[i] = (w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16]) << 1;
        if (w[i] & 0x80000000) {
            w[i] |= 0x1;
        }
    }

    a = context->state[0];
    b = context->state[1];
    c = context->state[2];
    d = context->state[3];
    e = context->state[4];

    for (int i = 0; i < 80; i++) {
        uint32_t f, k;
        if (i < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }

        uint32_t temp = ((a << 5) | (a >> 27)) + f + e + k + w[i];
        e = d;
        d = c;
        c = ((b << 30) | (b >> 2));
        b = a;
        a = temp;
    }

    context->state[0] += a;
    context->state[1] += b;
    context->state[2] += c;
    context->state[3] += d;
    context->state[4] += e;
}

void SHA1_Update(SHA1_CTX* context, const uint8_t* data, size_t len) {
    size_t i, j;
    j = (context->count[0] >> 3) & 63;
    if ((context->count[0] += len << 3) < (len << 3)) {
        context->count[1]++;
    }
    context->count[1] += (len >> 29);

    size_t part_len = 64 - j;
    if (len >= part_len) {
        memcpy(&context->buffer[j], data, part_len);
        SHA1_Transform(context, context->buffer);
        for (i = part_len; i + 63 < len; i += 64) {
            SHA1_Transform(context, &data[i]);
        }
        j = 0;
    } else {
        i = 0;
    }

    memcpy(&context->buffer[j], &data[i], len - i);
}

void SHA1_Final(SHA1_CTX* context, uint8_t digest[SHA1_DIGEST_SIZE]) {
    uint8_t padding[64] = { 0x80 };
    uint32_t i = (context->count[0] >> 3) & 63;
    uint32_t pad_len = (i < 56) ? (56 - i) : (120 - i);

    SHA1_Update(context, padding, pad_len);

    uint8_t length[8];
    for (int i = 0; i < 8; i++) {
        length[i] = (context->count[1] >> (56 - i * 8)) & 0xFF;
    }
    SHA1_Update(context, length, 8);

    for (int i = 0; i < 20; i++) {
        digest[i] = (context->state[i >> 2] >> (24 - (i % 4) * 8)) & 0xFF;
    }
}

void generateSHA1(const char* data, char* output) {
    SHA1_CTX context;
    SHA1_Init(&context);
    SHA1_Update(&context, (const uint8_t*)data, strlen(data));
    uint8_t digest[SHA1_DIGEST_SIZE];
    SHA1_Final(&context, digest);

    for (int i = 0; i < SHA1_DIGEST_SIZE; i++) {
        snprintf(output + i * 2, 3, "%02x", digest[i]);
    }
    output[40] = '\0';
}