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

int* get_buffer(const char* path, unsigned int* sizeOfBuffer); 

allErrors run(int* instructionsBuffer, const unsigned int size);