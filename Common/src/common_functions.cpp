#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "common_functions.h"


bool file_exist(FILE* stream) {
    return stream != NULL;   
}

long unsigned int get_file_size(FILE* stream) {
    struct stat buf = {};
    fstat(stream->_fileno, &buf);

    return buf.st_size;
}

char* get_name_stdin(const char* text) {
    char* name = NULL;

    if (text == NULL) {
        scanf("%ms[\n]", &name);

    } else {
        printf("%s", text);
        scanf("%ms[\n]", &name);
    }
    
    return name;
}

void processor_of_errors(allErrors error, const char* command, const int fileLine, const char* function, const char* name, const int line) { 
    switch (error) {

    case RETURNED_NULL_COMMON:
        printf("In file %s function %s line %d: calloc returned NULL\n", function, name, line);
        break;

    case FILE_WASNT_OPEN_COMMON:
        printf("In file %s function %s line %d: file wasn`t open\n", function, name, line);
        break;

    case NULLPTR_COMMON:
        printf("In file %s function %s line %d: NULL was given as a parameter\n", function, name, line);
        break;

    case SYNTAX_ERROR:
        printf("On line %d syntax error happened. Not found command \"%s\"\n", fileLine, command);
        break;

    case HLT_NOT_FOUND:
        printf("Syntax error happened not found \"hlt\"\n");
        break;

    case FEW_ARGS:
        printf("On line %d syntax error happened. Too few arguments to command \"%s\"\n", fileLine, command);
        break;

    case MANY_ARGS:
        printf("On line %d syntax error happened. Too many arguments to command \"%s\"\n", fileLine, command);
        break;

    case LABEL_ERROR:
        printf("On line %d syntax error happened. Wrong parameters given to jmp, or label doesn`t exist, or trying to rewrite existing label, or wrong format of label\n", fileLine);
        break;

    case WRONG_ARGS:
        printf("On line %d syntax error happened. Wrong arguments given to command \"%s\"\n", fileLine, command);
        break;

    case STACK_ERROR:
        printf("Error(s) in stack happened. Look at the Output_files/log.txt\n");
        break;

    case WRONG_EXE_FILE:
        printf("Given executable file cannot be executed. Check file and compiler version\n");
        break;

    case SEGMENTATION_FAULT:
        printf("In file %s function %s line %d: segmentation fault\n", function, name, line);
        break;

    case DEVISION_BY_ZERO:
        printf("In file %s function %s line %d: devision by zero\n", function, name, line);
        break;

    case WRONG_RAM_ARGS:
        printf("In file %s function %s line %d: ram error\n", function, name, line);
        break;

    case SOME_ERROR:
        printf("In file %s function %s line %d: something went wrong, programm finished\n", function, name, line);
        break;

    case WRONG_INPUT:
        printf("In file %s function %s line %d: wrong input\n", function, name, line);
        break;  

    case SQRT_ERROR:
        printf("In file %s function %s line %d: the square root of a negative number\n", function, name, line);
        break;

    default:
        return;
    }
}