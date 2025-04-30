
#include <stdio.h>
#include <string.h>
#include "showLogs.h"                
#include "checkHead.h"                

#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_RESET   "\033[0m"

//TODO: show logs according to branch name 

void showLogs(){
    char *branchName = headPath();
    char *refspath = refsheadPath();
    char logsPath[120];
    snprintf(logsPath,sizeof(logsPath),"./.bolt/logs/%s",refspath);
    FILE *fp = fopen(logsPath, "r");
    if (!fp) {
        perror("Failed to open logs file");
        return;
    }

    char line[512];
    printf("\n");

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "commit:", 7) == 0) {
            printf(COLOR_YELLOW "Commit: %s" COLOR_RESET, line + 7);
        } else if (strncmp(line, "author:", 7) == 0) {
            printf(COLOR_GREEN "Author: %s" COLOR_RESET, line + 7);
        } else if (strncmp(line, "author_mail:", 12) == 0) {
            printf(COLOR_GREEN "Email: %s" COLOR_RESET, line + 12);
        } else if (strncmp(line, "timestamp:", 10) == 0) {
            printf(COLOR_CYAN "Date: %s" COLOR_RESET, line + 10);
        } else if (strncmp(line, "message:", 8) == 0) {
            printf(COLOR_CYAN "Message: %s" COLOR_RESET, line + 8);
        } else if (strncmp(line, "----", 4) == 0) {
            printf("----------------------------------------\n");
        }
    }

    printf(COLOR_GREEN "Branch: %s" COLOR_RESET,branchName );

    fclose(fp);
}