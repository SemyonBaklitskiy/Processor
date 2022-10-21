#include "../common.h"

enum compiler_errors {
    NOERRORS            = 0,
    FILE_WASNT_OPEN     = 1,
    RETURNED_NULL       = 2,    // 1 << 1
    NULLPTR             = 4,
    SYNTAX_ERROR        = 8,
    HLT_NOT_FOUND       = 16,
    FEW_ARGS            = 32,
    MANY_ARGS           = 64,
    LABEL_ERROR         = 128,
    WRONG_ARGS          = 256,
 // 1 << 7;
};

struct table_label {
    int ip;
    const char* name; 
};

char* get_name_stdin(const char* text);

char* get_buffer(const char* path);

char** get_array_of_lines(char* buffer, int* amountOfLines);

int* get_exe_buffer(char** array, const int sizeOfArray, int* sizeOfExeBuffer);

compiler_errors get_exe_file(int* exeBuffer, const int sizeOfExeBuffer, const char* path);