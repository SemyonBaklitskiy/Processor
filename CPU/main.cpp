#include "cpu_functions.h"
#include <stdlib.h>

int main() {
    char* exeFilePath = get_name_stdin("Enter the executable file path: ");
    long unsigned int sizeOfBuffer = 0;

    char* instructionsBuffer = get_buffer(exeFilePath, &sizeOfBuffer);

    if (instructionsBuffer == NULL) 
        return -1;

    free(exeFilePath);

    allErrors error = run(instructionsBuffer, sizeOfBuffer);
    free(instructionsBuffer);

    return error;
}