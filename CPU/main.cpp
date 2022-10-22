#include "cpu_functions.h"
#include <stdlib.h>

int main() {
    char* exeFilePath = get_name_stdin("Enter the executable file path: ");
    unsigned int sizeOfBuffer = 0;

    int* instructionsBuffer = get_buffer(exeFilePath, &sizeOfBuffer);

    if (instructionsBuffer == NULL) 
        return -1;

    free(exeFilePath);

    cpu_errors error = run(instructionsBuffer, sizeOfBuffer);
    free(instructionsBuffer);

    if (error != NO_ERRORS_IN_CPU) 
        return -1;

    return 0;
}