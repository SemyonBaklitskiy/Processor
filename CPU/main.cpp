#include "cpu_functions.h"
#include <stdlib.h>

int main() {
    char* exeFilePath = get_name_stdin("Enter the executable file path: ");
    int* instructionsBuffer = get_buffer(exeFilePath);

    if (instructionsBuffer == NULL) 
        return -1;

    free(exeFilePath);

    cpu_errors error = run(instructionsBuffer);
    free(instructionsBuffer);

    if (error != NO_ERRORS_IN_CPU) 
        return -1;

    return 0;
}