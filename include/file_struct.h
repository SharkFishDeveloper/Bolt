#ifndef F_STRUCT_H
#define F_STRUCT_H

// Enum for file types
enum FILE_TYPE {
    FILE_TYPE_FILE, // File type value for files
    FILE_TYPE_DIR   // File type value for directories
};

// Struct to represent a file or directory
typedef struct F_STRUCT {
    char *file;        // Path to the file or directory
    enum FILE_TYPE type; // Type of file (either file or directory)
    char *sha1;        // SHA1 hash of the file
    int mode;
} F_STRUCT;

typedef struct F_STRUCT_ARRAY {
    F_STRUCT *files;  // Array of F_STRUCT elements
    int count;        // Number of files in the array
    int capacity;     // Capacity of the array
} F_STRUCT_ARRAY;


#endif
