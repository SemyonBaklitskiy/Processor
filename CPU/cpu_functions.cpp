#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "cpu_functions.h"
// TODO do not use relative pathes, use `g++ ... -I../Stack/includes` 

#define PRINT_ERROR(error) processor_of_errors(error, __FILE__, __PRETTY_FUNCTION__, __LINE__)

#define stack_destor(pointer) if (stack_destructor(pointer) != NO_IMPORTANT_ERRORS) { PRINT_ERROR(STACK_ERROR); return STACK_ERROR; }
#define stack_ctor(pointer, capacity) if (stack_constructor(pointer, capacity, VARNAME(pointer),__FILE__, __PRETTY_FUNCTION__, __LINE__) != NO_IMPORTANT_ERRORS) { PRINT_ERROR(STACK_ERROR); return NULL; } 
#define push(pointer, element) if (stack_push(pointer, element) != NO_IMPORTANT_ERRORS) { PRINT_ERROR(STACK_ERROR); return STACK_ERROR; } 
#define pop(pointer, element) if (stack_pop(pointer, element) != NO_IMPORTANT_ERRORS) { PRINT_ERROR(STACK_ERROR); return STACK_ERROR; }

static bool file_exist(FILE* stream);
static long int get_file_size(FILE* stream);
static bool correct_signature(FILE* file);
static struct my_cpu* cpu_constructor(const char* fileName, const char* functionName, unsigned int line);

static void processor_of_errors(cpu_errors error, const char* function, const char* name, const int line) { //TODO it's 3rd processor of errors))
    switch (error) {

    case RETURNED_NULL_IN_CPU:
        printf("In file %s function %s line %d: calloc returned NULL\n", function, name, line); // TODO: think about copypaste
        break;

    case FILE_WASNT_OPEN_IN_CPU:
        printf("In file %s function %s line %d: file wasn`t open\n", function, name, line);
        break;

    case NULLPTR_IN_CPU:
        printf("In file %s function %s line %d: NULL was given as a parameter\n", function, name, line);
        break;

    case STACK_ERROR:
        printf("Error(s) in stack happened. Look at the log.txt\n");
        break;

    case WRONG_EXE_FILE:
        printf("Given executable file cannot be executed. Check file and compiler version\n");
        break;

    case SEGMENTATION_FAULT:
        printf("In file %s function %s line %d: segmentation fault\n", function, name, line);
        break;

    case DEVIDE_BY_ZERO:
        printf("In file %s function %s line %d: devision by zero\n", function, name, line);
        break;

    default:
        return;
    }
}

static bool file_exist(FILE* stream) {
    return stream != NULL;   
}

static long int get_file_size(FILE* stream) {
    struct stat buf = {};
    fstat(stream->_fileno, &buf);

    return buf.st_size;
}

char* get_name_stdin(const char* text = NULL) { 
    char* name = NULL;

    if (text == NULL) {
        scanf("%ms[\n]", &name);

    } else {
        printf("%s", text);
        scanf("%ms[\n]", &name);
    }
    
    return name;
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

int* get_buffer(const char* path, unsigned int* sizeOfBuffer) {
    if (path == NULL) 
        PRINT_ERROR(NULLPTR_IN_CPU);

    FILE* file = fopen(path, "rb");

    if (!file_exist(file)) {
        PRINT_ERROR(FILE_WASNT_OPEN_IN_CPU);
        return NULL;
    }

    long int size = get_file_size(file);

    if (size == -1) 
        return NULL;

    if ((unsigned int)size < sizeof(SIGNATURE)) {
        PRINT_ERROR(WRONG_EXE_FILE);
        return NULL;
    }

    if (!correct_signature(file)) 
        return NULL;

    int* buffer = (int*)calloc(size - sizeof(SIGNATURE), sizeof(char)); 

    if (buffer == NULL) {
        PRINT_ERROR(RETURNED_NULL_IN_CPU);
        return NULL;
    }

    fread(buffer, sizeof(char), size - sizeof(SIGNATURE), file);
    *sizeOfBuffer = (size - sizeof(SIGNATURE)) / sizeof(elem_t);

    fclose(file);
    return buffer;
}

static struct my_cpu* cpu_constructor(const char* fileName, const char* functionName, unsigned int line) {
    struct my_cpu* cpu = (struct my_cpu*)calloc(1, sizeof(my_cpu));

    if (cpu == NULL) {
        PRINT_ERROR(RETURNED_NULL_IN_CPU);
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

cpu_errors run(int* instructionsBuffer, const unsigned int size) {
    if (instructionsBuffer == NULL) {
        PRINT_ERROR(NULLPTR_IN_CPU);
        return NULLPTR_IN_CPU;
    }

    unsigned int ip = 0;
    struct my_cpu* cpu = cpu_constructor(__FILE__, __PRETTY_FUNCTION__, __LINE__);
    const int sizeOfRam = sizeof(cpu->ram) / (sizeof(cpu->ram[0]));
    
    if (cpu == NULL) 
        return NULLPTR_IN_CPU;

    while ((instructionsBuffer[ip] != CMD_HLT) && (ip < size)) {
        if ((instructionsBuffer[ip] == (CMD_PUSH | ARG_IMMED)) || (instructionsBuffer[ip] == (CMD_PUSH | ARG_REG)) || (instructionsBuffer[ip] == (CMD_PUSH | ARG_REG | ARG_RAM)) || (instructionsBuffer[ip] == (CMD_PUSH | ARG_RAM | ARG_IMMED)) || (instructionsBuffer[ip] == (CMD_PUSH | ARG_RAM | ARG_REG | ARG_IMMED))) {
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

        } else if ((instructionsBuffer[ip] == (CMD_POP | ARG_IMMED)) || (instructionsBuffer[ip] == (CMD_POP | ARG_REG)) || (instructionsBuffer[ip] == (CMD_POP | ARG_REG | ARG_RAM)) || (instructionsBuffer[ip] == (CMD_POP | ARG_RAM | ARG_IMMED)) || (instructionsBuffer[ip] == (CMD_POP | ARG_RAM | ARG_REG | ARG_IMMED))) {
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

            if (secondElement == 0) {
                PRINT_ERROR(DEVIDE_BY_ZERO);
                return DEVIDE_BY_ZERO;
            }

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);
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
    return NO_ERRORS_IN_CPU;
}