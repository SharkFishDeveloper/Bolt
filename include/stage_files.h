
#ifndef STAGE_FILES_H
#define STAGE_FILES_H

#include "file_struct.h"
#include "ht.h"

F_STRUCT_ARRAY stageDirFiles(char *basepath,ht *map,int i);
void list_files_dir(char *basepath, F_STRUCT_ARRAY *file_array,ht *map);

#endif
            