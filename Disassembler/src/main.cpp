#include <stdlib.h>
#include "disassembler_functions.h"

int main() {
    char* exeFilePath = get_name_stdin("Enter path to the executable file: ");

    long unsigned int sizeOfExeBuffer = 0;
    
    char* exeBuffer = get_buffer(exeFilePath, &sizeOfExeBuffer);

    if (exeBuffer == NULL) {
        free(exeFilePath);
        return -1;
    }

    char* outFilePath = get_name_stdin("Enter path to the disassemble file: ");

    allErrors error = disassemble(exeBuffer, sizeOfExeBuffer, outFilePath);

    free(outFilePath);
    free(exeBuffer);

    return error;
}