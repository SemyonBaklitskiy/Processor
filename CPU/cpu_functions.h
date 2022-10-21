#include "../common.h"
#include "../Stack/includes/stack_functions.h"

enum cpu_errors {
    NO_ERRORS_IN_CPU = 1,
    FILE_WASNT_OPEN_IN_CPU = 2,
    RETURNED_NULL_IN_CPU = 4,
    NULLPTR_IN_CPU = 8,
    STACK_ERROR = 16,
    WRONG_EXE_FILE = 32,
    SEGMENTATION_FAULT = 64,
};

struct my_registers {
    const char* regName;
    elem_t value;
};

struct my_cpu {
    struct my_registers regs[5];
    struct stack st;
    elem_t ram[1000];

    const char* fileName;
    const char* functionName;
    unsigned int line;
};

char* get_name_stdin(const char* text);

int* get_buffer(const char* path);

cpu_errors run(int* instructionsBuffer);