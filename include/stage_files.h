
#ifndef STAGE_FILES_H
#define STAGE_FILES_H

#include "file_struct.h"

F_STRUCT_ARRAY stageDirFiles(char *basepath);
void list_files_dir(char *basepath, F_STRUCT_ARRAY *file_array);

#endif
            