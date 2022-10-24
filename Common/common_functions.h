#include <stdio.h>
#include "common.h"

bool file_exist(FILE* stream);

long unsigned int get_file_size(FILE* stream);

void processor_of_errors(allErrors error, const char* command, const int fileLine, const char* function, const char* name, const int line);

#define PRINT_ERROR(error) processor_of_errors(error, NULL, 0, __FILE__, __PRETTY_FUNCTION__, __LINE__)