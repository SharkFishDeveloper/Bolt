
#ifndef STAGE_FILES_STRUCT_H
#define STAGE_FILES_STRUCT_H

typedef struct STAGE_FILE_STRUCT {
    char **addedFiles;
    char **modedFiles;
    char **deletedFiles;
    int addedFileCount;
    int modedFileCount;
    int deletedFileCount;
    int addedFileCapacity;
    int modedFileCapacity;
    int deletedFileCapacity;
} STAGE_FILE_STRUCT;



#endif
            