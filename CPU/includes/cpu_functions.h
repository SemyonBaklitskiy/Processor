#include "common.h"
#include "stack_functions.h"

struct my_cpu {
    elem_t regs[amountOfRegs];
    struct stack st;
    struct stack stRet;
    elem_t ram[sizeOfRam];

    const char* fileName;
    const char* functionName;
    unsigned int line;
};

char* get_buffer(const char* path, long unsigned int* sizeOfBuffer); 
allErrors run(const char* instructionsBuffer, const long unsigned int size);