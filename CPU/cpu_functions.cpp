#include <stdlib.h>
#include <string.h>
#include "cpu_functions.h"
#include "../Common/common_functions.h"
// TODO do not use relative pathes, use `g++ ... -I../Stack/includes` 

#define stack_destor(pointer) if (stack_destructor(pointer) != NO_IMPORTANT_ERRORS) { PRINT_ERROR(STACK_ERROR); return STACK_ERROR; }
#define stack_ctor(pointer, capacity) if (stack_constructor(pointer, capacity, VARNAME(pointer),__FILE__, __PRETTY_FUNCTION__, __LINE__) != NO_IMPORTANT_ERRORS) { PRINT_ERROR(STACK_ERROR); return NULL; } 
#define push(pointer, element) if (stack_push(pointer, element) != NO_IMPORTANT_ERRORS) { PRINT_ERROR(STACK_ERROR); return STACK_ERROR; } 
#define pop(pointer, element) if (stack_pop(pointer, element) != NO_IMPORTANT_ERRORS) { PRINT_ERROR(STACK_ERROR); return STACK_ERROR; }
#define COMMAND_PUSH (instructionsBuffer[ip] == (CMD_PUSH | ARG_IMMED)) || (instructionsBuffer[ip] == (CMD_PUSH | ARG_REG)) || (instructionsBuffer[ip] == (CMD_PUSH | ARG_REG | ARG_RAM)) || (instructionsBuffer[ip] == (CMD_PUSH | ARG_RAM | ARG_IMMED)) || (instructionsBuffer[ip] == (CMD_PUSH | ARG_RAM | ARG_REG | ARG_IMMED))
#define COMMAND_POP (instructionsBuffer[ip] == (CMD_POP | ARG_IMMED)) || (instructionsBuffer[ip] == (CMD_POP | ARG_REG)) || (instructionsBuffer[ip] == (CMD_POP | ARG_REG | ARG_RAM)) || (instructionsBuffer[ip] == (CMD_POP | ARG_RAM | ARG_IMMED)) || (instructionsBuffer[ip] == (CMD_POP | ARG_RAM | ARG_REG | ARG_IMMED))

static bool correct_signature(FILE* file);
static struct my_cpu* cpu_constructor(const char* fileName, const char* functionName, unsigned int line);

int* get_buffer(const char* path, unsigned int* sizeOfBuffer) { 
    if (path == NULL) 
        PRINT_ERROR(NULLPTR_COMMON);

    FILE* file = fopen(path, "rb");

    if (!file_exist(file)) {
        PRINT_ERROR(FILE_WASNT_OPEN_COMMON);
        return NULL;
    }

    long int size = get_file_size(file);

    if ((unsigned int)size < sizeof(SIGNATURE)) {
        PRINT_ERROR(WRONG_EXE_FILE);
        return NULL;
    }

    if (!correct_signature(file)) 
        return NULL;

    int* buffer = (int*)calloc(size - sizeof(SIGNATURE), sizeof(char)); 

    if (buffer == NULL) {
        PRINT_ERROR(RETURNED_NULL_COMMON);
        return NULL;
    }

    fread(buffer, sizeof(char), size - sizeof(SIGNATURE), file);
    *sizeOfBuffer = (size - sizeof(SIGNATURE)) / sizeof(elem_t);

    fclose(file);
    return buffer;
}

allErrors run(int* instructionsBuffer, const unsigned int size) {
    if (instructionsBuffer == NULL) {
        PRINT_ERROR(NULLPTR_COMMON);
        return NULLPTR_COMMON;
    }

    unsigned int ip = 0;
    struct my_cpu* cpu = cpu_constructor(__FILE__, __PRETTY_FUNCTION__, __LINE__);
    
    if (cpu == NULL) 
        return NULLPTR_COMMON;

    while ((instructionsBuffer[ip] != CMD_HLT) && (ip < size)) {
        if (COMMAND_PUSH) {
            if (instructionsBuffer[ip] == (CMD_PUSH | ARG_IMMED | ARG_RAM)) {
                if ((instructionsBuffer[ip + 1] >= sizeOfRam) || (instructionsBuffer[ip + 1] < 0)) {
                    PRINT_ERROR(SEGMENTATION_FAULT);
                    return SEGMENTATION_FAULT;
                }

                push(&cpu->st, cpu->ram[instructionsBuffer[++ip]]);

            } else if (instructionsBuffer[ip] == (CMD_PUSH | ARG_IMMED)) {
                push(&cpu->st, instructionsBuffer[++ip]);

            } else if (instructionsBuffer[ip] == (CMD_PUSH | ARG_REG | ARG_RAM)) {
                if ((cpu->regs[instructionsBuffer[ip + 1]] >= sizeOfRam) || (cpu->regs[instructionsBuffer[ip + 1]] < 0)) {
                    PRINT_ERROR(SEGMENTATION_FAULT);
                    return SEGMENTATION_FAULT;
                }

                push(&cpu->st, cpu->ram[cpu->regs[instructionsBuffer[++ip]]]);

            } else if (instructionsBuffer[ip] == (CMD_PUSH | ARG_REG)) {
                push(&cpu->st, cpu->regs[instructionsBuffer[++ip]]);

            } else if (instructionsBuffer[ip] == (CMD_PUSH | ARG_RAM | ARG_REG | ARG_IMMED)) {
                if ((cpu->regs[instructionsBuffer[ip + 1]] + instructionsBuffer[ip + 2] >= sizeOfRam) || 
                    (cpu->regs[instructionsBuffer[ip + 1]] + instructionsBuffer[ip + 2] < 0)) {
                        
                    PRINT_ERROR(SEGMENTATION_FAULT);
                    return SEGMENTATION_FAULT;
                }

                push(&cpu->st, cpu->ram[cpu->regs[instructionsBuffer[ip + 1]] + instructionsBuffer[ip + 2]]);
                ip += 2;
            }

        } else if (COMMAND_POP) {
            if (instructionsBuffer[ip] == (CMD_POP | ARG_IMMED | ARG_RAM)) {
                elem_t element = 0;
                pop(&cpu->st, &element);

                if ((instructionsBuffer[ip + 1] >= sizeOfRam) || (instructionsBuffer[ip + 1] < 0)) {
                    PRINT_ERROR(SEGMENTATION_FAULT);
                    return SEGMENTATION_FAULT;
                }

                cpu->ram[instructionsBuffer[++ip]] = element;

            } else if (instructionsBuffer[ip] == (CMD_POP | ARG_IMMED)) {
                elem_t element = 0;
                pop(&cpu->st, &element);

            } else if (instructionsBuffer[ip] == (CMD_POP | ARG_REG | ARG_RAM)) {
                elem_t element = 0;
                pop(&cpu->st, &element);

                if ((cpu->regs[instructionsBuffer[ip + 1]] >= sizeOfRam) || (cpu->regs[instructionsBuffer[ip + 1]] < 0)) {
                    PRINT_ERROR(SEGMENTATION_FAULT);
                    return SEGMENTATION_FAULT;
                }

                cpu->ram[cpu->regs[instructionsBuffer[++ip]]] = element;

            } else if (instructionsBuffer[ip] == (CMD_POP | ARG_REG)) {
                elem_t element = 0;
                pop(&cpu->st, &element);
                cpu->regs[instructionsBuffer[++ip]] = element;

            } else if (instructionsBuffer[ip] == (CMD_POP | ARG_RAM | ARG_REG | ARG_IMMED)) {
                if ((cpu->regs[instructionsBuffer[ip + 1]] + instructionsBuffer[ip + 2] >= sizeOfRam) || 
                    (cpu->regs[instructionsBuffer[ip + 1]] + instructionsBuffer[ip + 2] < 0)) {

                    PRINT_ERROR(SEGMENTATION_FAULT);
                    return SEGMENTATION_FAULT;
                }

                elem_t element = 0;
                pop(&cpu->st, &element);

                cpu->ram[cpu->regs[instructionsBuffer[ip + 1]] + instructionsBuffer[ip + 2]] = element;
                ip += 2;
            }
            
        } else if (instructionsBuffer[ip] == CMD_ADD) {
            elem_t firstElement = 0;
            int secondElement = 0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);
            push(&cpu->st, firstElement + secondElement);

        } else if (instructionsBuffer[ip] == CMD_SUB) {
            elem_t firstElement = 0;
            int secondElement = 0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);
            push(&cpu->st, firstElement - secondElement);

        } else if (instructionsBuffer[ip] == CMD_MUL) {
            elem_t firstElement = 0;
            elem_t secondElement = 0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);
            push(&cpu->st, firstElement * secondElement);

        } else if (instructionsBuffer[ip] == CMD_DIV) {
            elem_t firstElement = 0;
            elem_t secondElement = 0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);

            if (secondElement == 0) {
                PRINT_ERROR(DEVISION_BY_ZERO);
                return DEVISION_BY_ZERO;
            }

            push(&cpu->st, firstElement / secondElement);

        } else if (instructionsBuffer[ip] == CMD_OUT) {
            elem_t element = 0;
            pop(&cpu->st, &element);
            printf("%d\n", element);

        } else if (instructionsBuffer[ip] == CMD_DUP) {
            elem_t element = 0;
            pop(&cpu->st, &element);
            push(&cpu->st, element);
            push(&cpu->st, element);

        } else if (instructionsBuffer[ip] == CMD_JMP) {
            ip = instructionsBuffer[ip + 1] - 1;

        } else if (instructionsBuffer[ip] == CMD_JB) {
            elem_t firstElement = 0;
            elem_t secondElement = 0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);

            if (firstElement < secondElement)
                ip = instructionsBuffer[ip + 1] - 1;

        } else if (instructionsBuffer[ip] == CMD_JBE) {
            elem_t firstElement = 0;
            elem_t secondElement = 0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);

            if (firstElement <= secondElement)
                ip = instructionsBuffer[ip + 1] - 1;

        } else if (instructionsBuffer[ip] == CMD_JA) {
            elem_t firstElement = 0;
            elem_t secondElement = 0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);

            if (firstElement > secondElement)
                ip = instructionsBuffer[ip + 1] - 1;

        } else if (instructionsBuffer[ip] == CMD_JAE) {
            elem_t firstElement = 0;
            elem_t secondElement = 0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);

            if (firstElement >= secondElement)
                ip = instructionsBuffer[ip + 1] - 1;

        } else if (instructionsBuffer[ip] == CMD_JE) {
            elem_t firstElement = 0;
            elem_t secondElement = 0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);

            if (firstElement == secondElement)
                ip = instructionsBuffer[ip + 1] - 1;

        } else if (instructionsBuffer[ip] == CMD_JNE) {
            elem_t firstElement = 0;
            elem_t secondElement = 0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);

            if (firstElement != secondElement)
                ip = instructionsBuffer[ip + 1] - 1;

        } else {

        }

        ++ip;
    }

    stack_destor(&cpu->st);
    free(cpu);
    return NOERRORS;
}

static struct my_cpu* cpu_constructor(const char* fileName, const char* functionName, unsigned int line) {
    struct my_cpu* cpu = (struct my_cpu*)calloc(1, sizeof(my_cpu));

    if (cpu == NULL) {
        PRINT_ERROR(RETURNED_NULL_COMMON);
        return NULL;
    }

    cpu->fileName = fileName;
    cpu->functionName = functionName;
    cpu->line = line;
    stack_ctor(&cpu->st, 2);
    
    for (long unsigned int regIndex = 0; regIndex < sizeof(cpu->regs) / sizeof(cpu->regs[0]); ++regIndex) 
        cpu->regs[regIndex] = 0;

    return cpu;
} 

static bool correct_signature(FILE* file) {
    elem_t fileSignature[3] = { 0, 0, 0 };
    fread(fileSignature, sizeof(char), sizeof(fileSignature), file);

    if ((fileSignature[0] != SIGNATURE[0]) || (fileSignature[1] != SIGNATURE[1]) || (fileSignature[2] > SIGNATURE[2])) {
        PRINT_ERROR(WRONG_EXE_FILE);
        return false;
    }

    return true;
}