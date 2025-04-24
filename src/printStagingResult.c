
#include <stdio.h>
#include "printStagingResult.h"                
#include "stage_file_struct.h"


#define RESET   "\x1b[0m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"


void printStagingResult(STAGE_FILE_STRUCT result) {
    printf("Changes to be committed:\n");

    if (result.addedFileCount > 0) {
        printf("  Added :\n");
        for (int i = 0; i < result.addedFileCount; i++) {
            printf("    " GREEN "+ %s" RESET "\n", result.addedFiles[i]);
        }
    }

    if (result.modedFileCount > 0) {
        printf("  Modified :\n");
        for (int i = 0; i < result.modedFileCount; i++) {
            printf("    " YELLOW "=> %s" RESET "\n", result.modedFiles[i]);
        }
    }

    if (result.deletedFileCount > 0) {
        printf("  Deleted :\n");
        for (int i = 0; i < result.deletedFileCount; i++) {
            printf("    " RED "- %s" RESET "\n", result.deletedFiles[i]);
        }
    }

    if (result.addedFileCount == 0 && result.modedFileCount == 0 && result.deletedFileCount == 0) {
        printf("  No changes detected.\n");
    }
}