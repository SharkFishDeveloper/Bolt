#include <stdlib.h>
#include "hexGenerator.h"    
#include <unistd.h>   
#include <time.h>
    
void generateRandomHex40(char *output) {
    const char hexDigits[] = "0123456789abcdef";
    
    srand(time(NULL));  // Improved randomness
    for (int i = 0; i < 40; i++) {
        output[i] = hexDigits[rand() % 16];
    }
    output[40] = '\0';
}