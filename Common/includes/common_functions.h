#include <stdio.h>
#include "common.h"

bool file_exist(FILE* stream);
long unsigned int get_file_size(FILE* stream);
void processor_of_errors(allErrors error, const char* command, const int fileLine, const char* function, const char* name, const int line);
bool correct_signature(FILE* file);
char* get_buffer(const char* path, long unsigned int* sizeOfBuffer);

#define PRINT_ERROR(error) processor_of_errors(error, NULL, 0, __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define INT_INSTRUCTIONS_BUFFER(ip) *((int*)(instructionsBuffer + ip)) 
#define ELEM_T_INSTRUCTIONS_BUFFER(ip) *((elem_t*)(instructionsBuffer + ip))
#define COMMAND_PUSH (instructionsBuffer[ip] == (CMD_PUSH | ARG_IMMED)) || (instructionsBuffer[ip] == (CMD_PUSH | ARG_REG)) || (instructionsBuffer[ip] == (CMD_PUSH | ARG_REG | ARG_RAM)) || (instructionsBuffer[ip] == (CMD_PUSH | ARG_RAM | ARG_IMMED)) || (instructionsBuffer[ip] == (CMD_PUSH | ARG_RAM | ARG_REG | ARG_IMMED))
#define COMMAND_POP (instructionsBuffer[ip] == (CMD_POP | ARG_IMMED)) || (instructionsBuffer[ip] == (CMD_POP | ARG_REG)) || (instructionsBuffer[ip] == (CMD_POP | ARG_REG | ARG_RAM)) || (instructionsBuffer[ip] == (CMD_POP | ARG_RAM | ARG_IMMED)) || (instructionsBuffer[ip] == (CMD_POP | ARG_RAM | ARG_REG | ARG_IMMED))

const int VERSION = 1; 
const int SIGNATURE[3] = { 'S', 'B', VERSION };

static const long unsigned int sizeOfInt = sizeof(int);
static const long unsigned int sizeOfElemt = sizeof(elem_t);