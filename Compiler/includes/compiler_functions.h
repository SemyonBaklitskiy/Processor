#include "common.h"

struct table_label {
    long int ip;
    const char* name; 
};

char* get_buffer(const char* path); 

char** get_array_of_lines(char* buffer, int* amountOfLines);

char* get_exe_buffer(char** array, const int sizeOfArray, long int* sizeOfExeBuffer);

allErrors get_exe_file(char* exeBuffer, const long int sizeOfExeBuffer, const char* path);