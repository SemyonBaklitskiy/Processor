#include <stdlib.h>
#include "disassembler_functions.h"
#include "common_functions.h"

static const char* reg_name(const int number);
static long unsigned int move_ip(const int command);
static void get_labels_ip(const char* instructionsBuffer, long unsigned int* labelsIp, const long unsigned int sizeOfBuffer, unsigned int* amountOfLabels);
static int comparator(const void* first, const void* second);
static int find_label(const long unsigned int* labelsIp, const unsigned int amountOfLabels, const long unsigned int label);

allErrors disassemble(const char* instructionsBuffer, const long unsigned int size, const char* outFilePath) {
    if ((instructionsBuffer == NULL) || (outFilePath == NULL)) { // TODO macro
        PRINT_ERROR(NULLPTR_COMMON);
        return NULLPTR_COMMON;
    }

    FILE* outFile = fopen(outFilePath, "w");
    if (!file_exist(outFile)) {
        PRINT_ERROR(FILE_WASNT_OPEN_COMMON);
        return FILE_WASNT_OPEN_COMMON;
    }

    long unsigned int ip = 0;
    unsigned int currentLabel = 0;

    long unsigned int labelsIp[sizeOfLabels] = {};

    unsigned int amountOfLabels = 0;
    get_labels_ip(instructionsBuffer, labelsIp, size, &amountOfLabels);
    qsort(labelsIp, amountOfLabels, sizeof(labelsIp[0]), comparator);

    while (ip < size) {
        if ((ip == labelsIp[currentLabel]) && (amountOfLabels != 0)) {
            fprintf(outFile, "%d:\n", currentLabel);
            currentLabel++;
        }

        if (COMMAND_PUSH) {
            if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_PUSH | ARG_IMMED | ARG_RAM)) {
                int number = INT_INSTRUCTIONS_BUFFER(ip + sizeOfInt);

                fprintf(outFile, "push [%d]\n", number);

            } else if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_PUSH | ARG_IMMED)) {
                fprintf(outFile, "push %lf\n", ELEM_T_INSTRUCTIONS_BUFFER(ip + sizeOfInt));

            } else if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_PUSH | ARG_REG | ARG_RAM)) {
                int number = INT_INSTRUCTIONS_BUFFER(ip + sizeOfInt);
                fprintf(outFile, "push [%s]\n", reg_name(number));

            } else if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_PUSH | ARG_REG)) {
                int number = INT_INSTRUCTIONS_BUFFER(ip + sizeOfInt);
                fprintf(outFile, "push %s\n", reg_name(number));

            } else if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_PUSH | ARG_RAM | ARG_REG | ARG_IMMED)) {
                int regNumber = INT_INSTRUCTIONS_BUFFER(ip + sizeOfInt);
                int number = INT_INSTRUCTIONS_BUFFER(ip + 2 * sizeOfInt);

                fprintf(outFile, "push [%s+%d]\n", reg_name(regNumber), number);
            }

        } else if (COMMAND_POP) {
            if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_POP | ARG_IMMED | ARG_RAM)) {
                int number = INT_INSTRUCTIONS_BUFFER(ip + sizeOfInt);

                fprintf(outFile, "pop [%d]\n", number);

            } else if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_POP | ARG_IMMED)) {
                fprintf(outFile, "pop\n");

            } else if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_POP | ARG_REG | ARG_RAM)) {
                int number = INT_INSTRUCTIONS_BUFFER(ip + sizeOfInt);

                fprintf(outFile, "pop [%s]\n", reg_name(number));

            } else if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_POP | ARG_REG)) {
                int number = INT_INSTRUCTIONS_BUFFER(ip + sizeOfInt);

                fprintf(outFile, "pop %s\n", reg_name(number));

            } else if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_POP | ARG_RAM | ARG_REG | ARG_IMMED)) {
                int regNumber = INT_INSTRUCTIONS_BUFFER(ip + sizeOfInt);
                int number = INT_INSTRUCTIONS_BUFFER(ip + 2 * sizeOfInt);

                fprintf(outFile, "pop [%s+%d]\n", reg_name(regNumber), number);
            }

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_ADD) {
            fprintf(outFile, "add\n");

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_SUB) {
            fprintf(outFile, "sub\n");

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_MUL) {
            fprintf(outFile, "mul\n");

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_DIV) {
            fprintf(outFile, "div\n");

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_OUT) {
            fprintf(outFile, "out\n");

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_DUP) {
            fprintf(outFile, "dup\n");

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_JMP) {
            fprintf(outFile, "jmp %d\n", find_label(labelsIp, amountOfLabels, INT_INSTRUCTIONS_BUFFER(ip + sizeOfInt)));

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_JB) {
            fprintf(outFile, "jb %d\n", find_label(labelsIp, amountOfLabels, INT_INSTRUCTIONS_BUFFER(ip + sizeOfInt)));

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_JBE) {
           fprintf(outFile, "jbe %d\n", find_label(labelsIp, amountOfLabels, INT_INSTRUCTIONS_BUFFER(ip + sizeOfInt)));

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_JA) {
            fprintf(outFile, "ja %d\n", find_label(labelsIp, amountOfLabels, INT_INSTRUCTIONS_BUFFER(ip + sizeOfInt)));

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_JAE) {
            fprintf(outFile, "jae %d\n", find_label(labelsIp, amountOfLabels, INT_INSTRUCTIONS_BUFFER(ip + sizeOfInt)));

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_JE) {
            fprintf(outFile, "je %d\n", find_label(labelsIp, amountOfLabels, INT_INSTRUCTIONS_BUFFER(ip + sizeOfInt)));

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_JNE) {
            fprintf(outFile, "jne %d", find_label(labelsIp, amountOfLabels, INT_INSTRUCTIONS_BUFFER(ip + sizeOfInt)));

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_CALL) {
            fprintf(outFile, "call %d\n", find_label(labelsIp, amountOfLabels, INT_INSTRUCTIONS_BUFFER(ip + sizeOfInt)));
    
        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_RET) {
            fprintf(outFile, "ret\n");

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_IN) {
            fprintf(outFile, "in\n");

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_SQRT) {
            fprintf(outFile, "sqrt\n");

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_HLT) {
            fprintf(outFile, "hlt\n");

        } else {
            PRINT_ERROR(SOME_ERROR);
            return SOME_ERROR;
        }

        ip += move_ip(INT_INSTRUCTIONS_BUFFER(ip));
    }

    fclose(outFile);
    return NOERRORS;
}

static const char* reg_name(const int number) {
    if (number == 0) { // TODO switch-case
        return "null";

    } else if (number == 1) {
        return "rax";

    } else if (number == 2) {
        return "rbx";

    } else if (number == 3) {
        return "rcx";

    } else {
        return "rdx";
    }
}

static void get_labels_ip(const char* instructionsBuffer, long unsigned int* labelsIp, const long unsigned int sizeOfBuffer, unsigned int* amountOfLabels) {
    long unsigned int ip = 0;
    unsigned int labelIndex = 0; 

    while (ip < sizeOfBuffer) {
        if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_JMP || INT_INSTRUCTIONS_BUFFER(ip) == CMD_JB || INT_INSTRUCTIONS_BUFFER(ip) == CMD_JBE ||
            INT_INSTRUCTIONS_BUFFER(ip) == CMD_JAE || INT_INSTRUCTIONS_BUFFER(ip) == CMD_JA || INT_INSTRUCTIONS_BUFFER(ip) == CMD_JNE || 
            INT_INSTRUCTIONS_BUFFER(ip) == CMD_JE  || INT_INSTRUCTIONS_BUFFER(ip) == CMD_CALL) {

            labelsIp[labelIndex] = INT_INSTRUCTIONS_BUFFER(ip + sizeOfInt);
            ++labelIndex;    
        }

        ip += move_ip(INT_INSTRUCTIONS_BUFFER(ip));
    }

    *amountOfLabels = labelIndex;
}

static long unsigned int move_ip(const int command) {
    if (command == (CMD_PUSH | ARG_IMMED | ARG_RAM) || command == (CMD_PUSH | ARG_REG | ARG_RAM) || command == (CMD_PUSH | ARG_REG)) {
        return 2 * sizeOfInt;

    } else if (command == (CMD_PUSH | ARG_IMMED)) {
        return sizeOfInt + sizeOfElemt;

    } else if (command == (CMD_PUSH | ARG_RAM | ARG_REG | ARG_IMMED) || command == (CMD_POP | ARG_RAM | ARG_REG | ARG_IMMED)) {
        return 3 * sizeOfInt;

    } else if (command == (CMD_POP | ARG_IMMED | ARG_RAM) || command == (CMD_POP | ARG_REG | ARG_RAM) || command == (CMD_POP | ARG_REG)) {
        return 2 * sizeOfInt;

    } else if (command == CMD_JMP || command == CMD_JB || command == CMD_JBE ||
               command == CMD_JAE || command == CMD_JA || command == CMD_JNE || 
               command == CMD_JE  || command == CMD_CALL) {

        return 2 * sizeOfInt; 

    } else {
        return sizeOfInt;
    }
}

static int comparator(const void* first, const void* second) {
    return *((long unsigned int*)first) - *((long unsigned int*)second);
}

static int find_label(const long unsigned int* labelsIp, const unsigned int amountOfLabels, const long unsigned int label) {
    for (unsigned int labelIndex = 0; labelIndex < amountOfLabels; ++labelIndex) {
        if (label == labelsIp[labelIndex])
            return labelIndex;
    }

    return -1;
}

