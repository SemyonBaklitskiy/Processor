#include "../Common/common.h"

struct table_label {
    int ip;
    const char* name; 
};

char* get_buffer(const char* path); //mb common

char** get_array_of_lines(char* buffer, int* amountOfLines);

int* get_exe_buffer(char** array, const int sizeOfArray, int* sizeOfExeBuffer);

allErrors get_exe_file(int* exeBuffer, const int sizeOfExeBuffer, const char* path);