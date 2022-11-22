#include "common.h"

char* get_buffer(const char* path, long unsigned int* sizeOfBuffer);
allErrors disassemble(const char* exeBuffer, const long unsigned int size, const char* outFilePath);