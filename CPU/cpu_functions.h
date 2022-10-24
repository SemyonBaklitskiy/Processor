#include "../Common/common.h"
#include "../Stack/includes/stack_functions.h"

struct my_cpu {
    elem_t regs[amountOfRegs];
    struct stack st;
    elem_t ram[sizeOfRam];

    const char* fileName;
    const char* functionName;
    unsigned int line;
};

char* get_buffer(const char* path, long unsigned int* sizeOfBuffer); 

allErrors run(char* instructionsBuffer, const long unsigned int size);