
#ifndef GOTOPREVCOMMITID_H
#define GOTOPREVCOMMITID_H

void gotoPreviousCommitId(char *commitId,int check);
void removeFileAndDeleteEmptyDirs(const char *filePath);
void removeEmptyDirsUpward(const char *path);
#endif
            