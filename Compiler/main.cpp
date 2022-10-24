#include <stdio.h>
#include <stdlib.h>
#include "compiler_functions.h"

#define CHECK(firstPointer, secondPointer, ...) if (firstPointer == NULL) { free(secondPointer); __VA_ARGS__; return -1; }
#define GET_EXE_FILE(array, size, path) allErrors error = get_exe_file(array, size, path); if (error != NOERRORS) { free(exeFilePath); free(exeBuffer); return -1; }

int main() {
    char* sourceFilePath = get_name_stdin("Enter path to the source file: ");

    char* buffer = get_buffer(sourceFilePath);
    CHECK(buffer, sourceFilePath)

    free(sourceFilePath);

    int amountOfLines = 0;
    char** arrayOfLines = get_array_of_lines(buffer, &amountOfLines);
    CHECK(arrayOfLines, buffer)

    long int sizeOfExeBuffer = 0;
    char* exeBuffer = get_exe_buffer(arrayOfLines, amountOfLines, &sizeOfExeBuffer);
    CHECK(exeBuffer, arrayOfLines, free(buffer))

    free(arrayOfLines);
    free(buffer);

    char* exeFilePath = get_name_stdin("Enter path to the executable file: ");
    GET_EXE_FILE(exeBuffer, sizeOfExeBuffer, exeFilePath);

    free(exeFilePath);
    free(exeBuffer);

    printf("Success!\n");
    return 0;
}