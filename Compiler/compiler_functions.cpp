#include "compiler_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <cctype>

//TODO NO COPYPAST

static const int version = 1;

static const int signature[3] = { 'S', 'B', version };

static const char* regNames[] = {"null", "rax", "rbx", "rcx", "rdx" };

static const unsigned int sizeOfLabels = 10;

static const int sizeOfRam = 1000;

static unsigned int currentLabel = 0;

static unsigned const int sizeOfCommands = 25;

static struct table_label labels[sizeOfLabels];

static void processor_of_errors(compiler_errors error, const char* command, const int fileLine, const char* function, const char* name, const int line) { 
    switch (error) {

    case RETURNED_NULL:
        printf("In file %s function %s line %d: calloc returned NULL\n", function, name, line);
        break;

    case FILE_WASNT_OPEN:
        printf("In file %s function %s line %d: file wasn`t open\n", function, name, line);
        break;

    case NULLPTR:
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

    default:
        return;
    }
}

#define PRINT_ERROR(error) processor_of_errors(error, NULL, 0, __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define compilation_errors(error, command, line) processor_of_errors(error, command, line, __FILE__, __PRETTY_FUNCTION__, __LINE__)

static void fill_labels(const int poison, const char* name) {
    if (name == NULL) {
        PRINT_ERROR(NULLPTR);
        return;
    }

    for (unsigned int labelsIndex = 0; labelsIndex < sizeOfLabels; ++labelsIndex) {
        labels[labelsIndex].ip = poison;
        labels[labelsIndex].name = name;
    }
}

static bool file_is_open(FILE* stream) { //TODO use onegin lib
    if (stream == NULL) 
        return false;

    return true;   
}

static long int get_file_size(FILE* stream) {
    if (stream == NULL) {
        PRINT_ERROR(NULLPTR);
        return -1;
    }

    struct stat buf;
    fstat(stream->_fileno, &buf);

    return buf.st_size / sizeof(char);
}

static int amount_of_lines(char* buffer) {
    if (buffer == NULL) {
        PRINT_ERROR(NULLPTR);
        return -1;
    }

    int amount = 0; 

    for (unsigned int bufferPointer = 0; buffer[bufferPointer] != '\0'; ++bufferPointer) {
        if (buffer[bufferPointer] == '\n') {
            ++amount;
        }
    }

    return amount + 1;
}

static bool string_is_empty(const char* str) {
    if (str == NULL) {
        PRINT_ERROR(NULLPTR);
        return false;
    }

    for (int strIndex = 0; str[strIndex] != '\0'; ++strIndex) {
        if (!isspace(str[strIndex]))
            return false; 
    }

    return true;
}

static int find_label_in_table(const char* labelName) {
    if (labelName == NULL) {
        PRINT_ERROR(NULLPTR);
        return -1;
    }

    for (unsigned int labelsIndex = 0; labelsIndex < sizeOfLabels; ++labelsIndex) {
        if (strcmp(labelName, labels[labelsIndex].name) == 0) 
            return labels[labelsIndex].ip;     
    }

    return -1;
}

static int get_label(char* labelName) {
    if (labelName == NULL) {
        PRINT_ERROR(NULLPTR);
        return -2;
    }

    if (labelName[0] == ':')
        ++labelName;

    unsigned int numberOfLabel = 0;

    int check = sscanf(labelName, "%d ", &numberOfLabel);

    if (check <= 0) {
        char nameOfLabel[sizeOfCommands] = "";
        sscanf(labelName, "%s ", nameOfLabel);
        return find_label_in_table(nameOfLabel);

    } else {
        if (numberOfLabel < sizeOfLabels)
            return labels[numberOfLabel].ip;

        return -1;
    }
    
}

static int get_jmp_args(const char* args) {
    if (args == NULL) {
        PRINT_ERROR(NULLPTR);
        return -2;
    }

    unsigned int numberOfLabel = 0;

    if (string_is_empty(args)) 
        return -2;

    if (sscanf(args, " %d ", &numberOfLabel) <= 0) {
        char labelName[sizeOfCommands] = "";
        sscanf(args, " %s ", labelName);

        return get_label(labelName);

    } else {
        if (numberOfLabel < sizeOfLabels)
            return labels[numberOfLabel].ip;
        
        return -1;
    }
}

static bool is_label(const char* str) {
    if (str == NULL) {
        PRINT_ERROR(NULLPTR);
        return false;
    }

    if ((str[strlen(str) - 1] != ':') || (strlen(str) <= 1))
        return false;

    return true;
}

static int make_label(char* str, const int ip) {
    if (str == NULL) {
        PRINT_ERROR(NULLPTR);
        return -1;
    }

    str[strlen(str) - 1] = '\0';

    while (isspace(*str))
        ++str;

    int numberOfLabel = 0;

    int check = sscanf(str, "%d", &numberOfLabel);

    if (check > 0) {
        if ((numberOfLabel < 0) || (labels[numberOfLabel].ip != -1)) 
            return -1;

        labels[numberOfLabel].ip = ip;

    } else {
        if (find_label_in_table(str) != -1)
            return -1;

        for (currentLabel = 0; currentLabel < sizeOfLabels; ++currentLabel) {
            if (labels[currentLabel].ip == -1)
                break;
        }

        if ((currentLabel == sizeOfLabels - 1) && (labels[currentLabel].ip != -1))
            return -1;

        labels[currentLabel].name = str;
        labels[currentLabel].ip = ip;
        ++currentLabel;
    }

    return 0;
}

static bool correct_forward_labels(char** array, const int sizeOfArray) {
    if (array == NULL) {
        PRINT_ERROR(NULLPTR);
        return false;
    }

    bool errorFound = false;

    for (int arrayIndex = 0; arrayIndex < sizeOfArray; ++arrayIndex) {
        char command[sizeOfCommands] = "";
        int pos = 0;

        sscanf(array[arrayIndex], " %s %n ", command, &pos);

        if ((strcmp(command, "jmp") == 0) || (strcmp(command, "jb") == 0) || (strcmp(command, "jbe") == 0) || (strcmp(command, "ja") == 0) || (strcmp(command, "jae") == 0)
         || (strcmp(command, "je") == 0) || (strcmp(command, "jne") == 0)) {
            int arg = get_jmp_args(array[arrayIndex] + pos);

            if (arg == -1) {
                compilation_errors(LABEL_ERROR, command, arrayIndex + 1);
                errorFound = true;
            }
        }
    }  

    return !errorFound;
}

static int reg_number(const char* argument) {
    if (argument == NULL) {
        PRINT_ERROR(NULLPTR);
        return -1;
    }

    if (strcmp(argument, "rax") == 0) {
        return REG_RAX;

    } else if (strcmp(argument, "rbx") == 0) {
        return REG_RBX;

    } else if (strcmp(argument, "rcx") == 0) {
        return REG_RCX;

    } else if (strcmp(argument, "rdx") == 0) {
        return REG_RDX;

    } else if (strcmp(argument, "null") == 0) {
        return REG_NULL;

    } else {
        return -1;  
    }
    
}

static bool check_all_options(char* argument) {
    if (argument == NULL) {
        PRINT_ERROR(NULLPTR);
        return false;
    }
    
    char* posOfPlus = strchr(argument, '+');

    if ((posOfPlus == NULL) || (posOfPlus == argument)) 
        return false;  

    char str2[sizeOfCommands] = "";
    char str1[sizeOfCommands] = "";

    int pos = 0;

    sscanf(posOfPlus + 1, " %s %n", str2, &pos); 
    posOfPlus[0] = '\0';
    sscanf(argument, " %s ", str1);

    if (!string_is_empty(posOfPlus + 1 + pos)) 
        return false;

    int number = 0;
    int nextPos = 0;
    int isNumberStr1 = sscanf(str1, " %d %n", &number, &nextPos);
    int isNumberStr2 = sscanf(str2, " %d %n", &number, &nextPos);

    if ((isNumberStr1 > 0) && (isNumberStr2 > 0)) {
        return false;

    } else if ((isNumberStr1 <= 0) && (isNumberStr2 <= 0)) {
        return false;

    } else if ((isNumberStr1 > 0) && (string_is_empty(str1 + nextPos))) {
        int regNumber = reg_number(str2);

        if (regNumber == -1)
            return false;

    } else if ((isNumberStr2 > 0) && (string_is_empty(str2 + nextPos))) {
        int regNumber = reg_number(str1);

        if (regNumber == -1)
            return false;

    } else {
        return false;
    }

    posOfPlus[0] = '+';

    return true;
}

static bool correct_push_and_pop_argument(char* argument, int* size) { 
    if ((argument == NULL) || (size == NULL)) {
        PRINT_ERROR(NULLPTR);
        return false;
    }

    if ((argument[0] == '[') && (argument[strlen(argument) - 1] == ']')) {
        argument[strlen(argument) - 1] = '\0';
        ++argument;

        bool allOptions = check_all_options(argument);

        if (allOptions) {
            *(size) += 1;
            return true;
        }
    }

    int arg = 0;
    int pos = 0;
    int isNumber = sscanf(argument, " %d %n ", &arg, &pos);

    if ((isNumber > 0) && (arg >= 0) && (arg < sizeOfRam) && (string_is_empty(argument + pos))) 
        return true;
    
    for (long unsigned int regIndex = 0; regIndex < sizeof(regNames) / sizeof(regNames[0]); ++regIndex) {
        if (strcmp(argument, regNames[regIndex]) == 0)
            return true;
    }

    return false;
}

static int get_reg_number_and_immed(char* argument, int* immed, int* regNumber) {
    if ((argument == NULL) || (immed == NULL) || (regNumber == NULL)) {
        PRINT_ERROR(NULLPTR);
        return -1;
    }

    char* posOfPlus = strchr(argument, '+');
    posOfPlus[0] = '\0';

    char str1[sizeOfCommands] = "";
    char str2[sizeOfCommands] = "";

    sscanf(argument, " %s ", str1);
    sscanf(posOfPlus + 1, " %s ", str2);

    int number = 0;
    int isNumberStr1 = sscanf(str1, " %d ", &number);

    if (isNumberStr1 > 0) {
        *immed = number;
        *regNumber = reg_number(str2);

    } else {
        sscanf(str2, " %d ", &number);
        *immed = number;
        *regNumber = reg_number(str1);
    }

    return 0;
}

static int exe_buffer_size_and_check(char** array, const int sizeOfArray) {
    if (array == NULL) {
        PRINT_ERROR(NULLPTR);
        return -1;
    }

    fill_labels(-1, "Noname");

    int size = 0;
    bool hltFound = false;
    bool errorFound = false;
    bool hasForwardLabels = false;

    for (int arrayIndex = 0; arrayIndex < sizeOfArray; ++arrayIndex) {
        char command[sizeOfCommands] = "";
        int pos = 0;

        sscanf(array[arrayIndex], " %s %n ", command, &pos);

        if (strcmp(command, "push") == 0) {
            size += 2; 
            int argument = 0;

            int nextPos = 0;
            int checkForArgs = sscanf(array[arrayIndex] + pos, " %d %n", &argument, &nextPos);

            if (string_is_empty(array[arrayIndex] + pos)) {
                compilation_errors(FEW_ARGS, command, arrayIndex + 1);
                errorFound = true;
                
            } else if (checkForArgs <= 0) {
                char arg[sizeOfCommands] = "";

                sscanf(array[arrayIndex] + pos, " %s %n", arg, &nextPos);

                if (!string_is_empty(array[arrayIndex] + pos + nextPos)) {
                    compilation_errors(MANY_ARGS, command, arrayIndex + 1);
                    errorFound = true;

                } else if (!correct_push_and_pop_argument(array[arrayIndex] + pos, &size)) {
                    compilation_errors(WRONG_ARGS, command, arrayIndex + 1);
                    errorFound = true;

                } else {
                    continue;
                }
                
            }   else if ((checkForArgs > 0) && !(string_is_empty(array[arrayIndex] + pos + nextPos))) {
                compilation_errors(WRONG_ARGS, command, arrayIndex + 1);
                errorFound = true;
            }

        } else if (strcmp(command, "pop") == 0) {
            int argument = 0;

            int nextPos = 0;
            int checkForArgs = sscanf(array[arrayIndex] + pos, " %d %n", &argument, &nextPos);

            if (string_is_empty(array[arrayIndex] + pos)) {
                ++size;
                continue;
                
            } else if (checkForArgs <= 0)  {
                char arg[sizeOfCommands] = "";

                sscanf(array[arrayIndex] + pos, " %s %n", arg, &nextPos);

                if (!string_is_empty(array[arrayIndex] + pos + nextPos)) {
                    compilation_errors(MANY_ARGS, command, arrayIndex + 1);
                    errorFound = true;

                } else if (!correct_push_and_pop_argument(array[arrayIndex] + pos, &size)) {
                    compilation_errors(WRONG_ARGS, command, arrayIndex + 1);
                    errorFound = true;
                    
                } else {
                    size += 2;
                    continue;
                }
                
            }   else {
                compilation_errors(WRONG_ARGS, command, arrayIndex + 1);
                errorFound = true;
            }

        } else if (strcmp(command, "add") == 0) {
            ++size;

            if (!string_is_empty(array[arrayIndex] + pos)) {
                compilation_errors(MANY_ARGS, command, arrayIndex + 1);
                errorFound = true;
            }

        } else if (strcmp(command, "sub") == 0) {
            ++size;

            if (!string_is_empty(array[arrayIndex] + pos)) {
                compilation_errors(MANY_ARGS, command, arrayIndex + 1);
                errorFound = true;
            }

        } else if (strcmp(command, "mul") == 0) {
            ++size;

            if (!string_is_empty(array[arrayIndex] + pos)) {
                compilation_errors(MANY_ARGS, command, arrayIndex + 1);
                errorFound = true;
            }

        } else if (strcmp(command, "div") == 0) {
            ++size;

            if (!string_is_empty(array[arrayIndex] + pos)) {
                compilation_errors(MANY_ARGS, command, arrayIndex + 1);
                errorFound = true;
            }

        } else if (strcmp(command, "out") == 0) {
            ++size;

            if (!string_is_empty(array[arrayIndex] + pos)) { 
                compilation_errors(MANY_ARGS, command, arrayIndex + 1);
                errorFound = true;
            }

        } else if (strcmp(command, "dup") == 0) {
            ++size;

            if (!string_is_empty(array[arrayIndex] + pos)) { 
                compilation_errors(MANY_ARGS, command, arrayIndex + 1);
                errorFound = true;
            }

        } else if (strcmp(command, "jmp") == 0) {
            go_to_label:

            size += 2;

            int arg = get_jmp_args(array[arrayIndex] + pos);
            char argName[sizeOfCommands] = "";
            int nextPos = 0;

            sscanf(array[arrayIndex] + pos, " %s %n", argName, &nextPos);

            if (!string_is_empty(array[arrayIndex] + pos + nextPos)) {
                compilation_errors(MANY_ARGS, command, arrayIndex + 1);
                errorFound = true;

            } else if (arg == -1) {
                hasForwardLabels = true;
                continue;

            } else if (arg == -2) {
                compilation_errors(FEW_ARGS, command, arrayIndex + 1);
                errorFound = true;
            }  

        } else if ((strcmp(command, "jb") == 0) || (strcmp(command, "jbe") == 0) || (strcmp(command, "ja") == 0) || (strcmp(command, "jae") == 0)
                || (strcmp(command, "je") == 0) || (strcmp(command, "jne") == 0)) {
            goto go_to_label;

        } else if (strcmp(command, "hlt") == 0) {
            ++size;
            hltFound = true;

            if (!string_is_empty(array[arrayIndex] + pos)) {
                compilation_errors(MANY_ARGS, command, arrayIndex + 1);
                errorFound = true;
                hltFound = true;
            }

        } else if (is_label(command)) {
            if (!string_is_empty(array[arrayIndex] + pos)) {
                compilation_errors(MANY_ARGS, command, arrayIndex + 1);
                errorFound = true;

            } else if (make_label(array[arrayIndex], size) == -1) {
                compilation_errors(LABEL_ERROR, command, arrayIndex + 1);
                errorFound = true;
            } 

        } else if (string_is_empty(array[arrayIndex])) {
            continue;

        } else {
            compilation_errors(SYNTAX_ERROR, command, arrayIndex + 1);
            errorFound = true;
        }
    }

    if (!hltFound) {
        compilation_errors(HLT_NOT_FOUND, "hlt", 0);
        errorFound = true;
    }

    bool forwardLabelsError = false;

    if (hasForwardLabels) 
        forwardLabelsError = !correct_forward_labels(array, sizeOfArray);

    if (errorFound || forwardLabelsError)
        return -1;

    return size;
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

char* get_buffer(const char* path) {
    if (path == NULL) 
        PRINT_ERROR(NULLPTR);
    

    FILE* file = fopen(path, "r");

    if (file == NULL) {
        PRINT_ERROR(FILE_WASNT_OPEN);
        return NULL;
    }

    long int size = get_file_size(file);

    if (size == -1) {
        return NULL;
    }

    char* buffer = (char*)calloc(size + 1, sizeof(char));

    if (buffer == NULL) {
        PRINT_ERROR(RETURNED_NULL);
        return NULL;
    }

    fread(buffer, sizeof(char), size, file);
    buffer[size] = '\0';

    fclose(file);
    return buffer;
}

char** get_array_of_lines(char* buffer, int* amountOfLines) {
    if (buffer == NULL || amountOfLines == NULL) {
        PRINT_ERROR(NULLPTR);
        return NULL;
    }

    int amount = amount_of_lines(buffer);

    if (amount == -1)
        return NULL;

    *amountOfLines = amount;

    char** array = (char**)calloc(*amountOfLines, sizeof(char*));

    if (array == NULL) {
        PRINT_ERROR(RETURNED_NULL);
        return NULL;
    }

    array[0] = &buffer[0];

    unsigned int arrayPointer = 1;

    for (unsigned int bufferPointer = 0; buffer[bufferPointer] != '\0'; ++bufferPointer) {
        if (buffer[bufferPointer] == '\n') {
            buffer[bufferPointer] = '\0';
            array[arrayPointer] = &buffer[bufferPointer + 1];
            ++arrayPointer;
        }

        if (buffer[bufferPointer] == ';')
            buffer[bufferPointer] = '\0';
    }

    return array;
}

int* get_exe_buffer(char** array, const int sizeOfArray, int* sizeOfExeBuffer) {
    if ((array == NULL) || (sizeOfExeBuffer == NULL)) {
        PRINT_ERROR(NULLPTR);
        return NULL;
    }

    int size = exe_buffer_size_and_check(array, sizeOfArray);

    if (size == -1)
        return NULL;

    int* exeBuffer = (int*)calloc(size, sizeof(int));

    if (exeBuffer == NULL) {
        PRINT_ERROR(RETURNED_NULL);
        return NULL;
    }

    *sizeOfExeBuffer = size;

    unsigned int bufferIndex = 0;

    for (int arrayIndex = 0; arrayIndex < sizeOfArray; ++arrayIndex) {
        char command[25] = "";
        int pos = 0;

        sscanf(array[arrayIndex], " %s %n ", command, &pos);

        if (strcmp(command, "push") == 0) {
            int argument = 0;

            if (sscanf(array[arrayIndex] + pos, " %d ", &argument) > 0) {
                exeBuffer[bufferIndex++] = CMD_PUSH | ARG_IMMED;
                exeBuffer[bufferIndex++] = argument;

           } else {
                char arg[sizeOfCommands] = "";
                sscanf(array[arrayIndex] + pos, " %s ", arg);

                bool ram = false;
                bool allOptions = false;
                int options = 0;
                int val = 0;
                int immed = 0;


                if (arg[0] == '[') 
                    ram = true;

                int regNumber = -2;

                if (ram) {
                    int ramNumber = 0;
                    int isNumber = sscanf(arg + 1, " %d ", &ramNumber);

                    if (check_all_options(arg + 1)) {
                        allOptions = true;
                        options |= (CMD_PUSH | ARG_RAM | ARG_REG | ARG_IMMED);

                        if (get_reg_number_and_immed(arg + 1, &immed, &regNumber) == -1)
                            return NULL;

                        val = regNumber;

                    } else if (isNumber > 0) {
                        options |= (CMD_PUSH | ARG_RAM | ARG_IMMED);
                        val = ramNumber;

                    } else {
                        regNumber = reg_number(arg + 1);
                        options |= (CMD_PUSH | ARG_RAM | ARG_REG);
                        val = regNumber;
                    }

                } else {
                    regNumber = reg_number(arg);
                    options |= (CMD_PUSH | ARG_REG);
                    val = regNumber;
                }

                if (regNumber == -1) 
                    return NULL;

                if (allOptions) {
                    exeBuffer[bufferIndex++] = options;
                    exeBuffer[bufferIndex++] = val;
                    exeBuffer[bufferIndex++] = immed;

                } else {
                    exeBuffer[bufferIndex++] = options;
                    exeBuffer[bufferIndex++] = val;
                }
           }

        } else if (strcmp(command, "pop") == 0) {
            if (string_is_empty(array[arrayIndex] + pos)) {
                exeBuffer[bufferIndex++] = CMD_POP | ARG_IMMED;

            } else {
               char arg[sizeOfCommands] = "";
                sscanf(array[arrayIndex] + pos, " %s ", arg);

                bool ram = false;
                bool allOptions = false;
                int options = 0;
                int val = 0;
                int immed = 0;


                if (arg[0] == '[') 
                    ram = true;

                int regNumber = -2;

                if (ram) {
                    int ramNumber = 0;
                    int isNumber = sscanf(arg + 1, " %d ", &ramNumber);

                    if (check_all_options(arg + 1)) {
                        allOptions = true;
                        options |= (CMD_POP | ARG_RAM | ARG_REG | ARG_IMMED);

                        if (get_reg_number_and_immed(arg + 1, &immed, &regNumber) == -1)
                            return NULL;

                        val = regNumber;

                    } else if (isNumber > 0) {
                        options |= (CMD_POP | ARG_RAM | ARG_IMMED);
                        val = ramNumber;

                    } else {
                        regNumber = reg_number(arg + 1);
                        options |= (CMD_POP | ARG_RAM | ARG_REG);
                        val = regNumber;
                    }

                } else {
                    regNumber = reg_number(arg);
                    options |= (CMD_POP | ARG_REG);
                    val = regNumber;
                }

                if (regNumber == -1) 
                    return NULL;

                if (regNumber == REG_NULL) {
                    compilation_errors(WRONG_ARGS, command, arrayIndex + 1);
                    return NULL;
                }

                if (allOptions) {
                    exeBuffer[bufferIndex++] = options;
                    exeBuffer[bufferIndex++] = val;
                    exeBuffer[bufferIndex++] = immed;
                    
                } else {
                    exeBuffer[bufferIndex++] = options;
                    exeBuffer[bufferIndex++] = val;
                }
            }

        } else if (strcmp(command, "add") == 0) {
            exeBuffer[bufferIndex++] = CMD_ADD;

        } else if (strcmp(command, "sub") == 0) {
            exeBuffer[bufferIndex++] = CMD_SUB;

        } else if (strcmp(command, "mul") == 0) {
            exeBuffer[bufferIndex++] = CMD_MUL;

        } else if (strcmp(command, "div") == 0) {
            exeBuffer[bufferIndex++] = CMD_DIV;

        } else if (strcmp(command, "out") == 0) {
            exeBuffer[bufferIndex++] = CMD_OUT;

        } else if (strcmp(command, "hlt") == 0) {
            exeBuffer[bufferIndex++] = CMD_HLT;

        } else if (strcmp(command, "dup") == 0) {
            exeBuffer[bufferIndex++] = CMD_DUP;

        } else if (strcmp(command, "jmp") == 0) {
            int arg = get_jmp_args(array[arrayIndex] + pos);

            exeBuffer[bufferIndex++] = CMD_JMP;
            exeBuffer[bufferIndex++] = arg; 

        } else if (strcmp(command, "jb") == 0) {
            int arg = get_jmp_args(array[arrayIndex] + pos);

            exeBuffer[bufferIndex++] = CMD_JB;
            exeBuffer[bufferIndex++] = arg;

        } else if (strcmp(command, "jbe") == 0) {
            int arg = get_jmp_args(array[arrayIndex] + pos);

            exeBuffer[bufferIndex++] = CMD_JBE;
            exeBuffer[bufferIndex++] = arg;

        } else if (strcmp(command, "ja") == 0) {
            int arg = get_jmp_args(array[arrayIndex] + pos);

            exeBuffer[bufferIndex++] = CMD_JA;
            exeBuffer[bufferIndex++] = arg;

        } else if (strcmp(command, "jae") == 0) {
            int arg = get_jmp_args(array[arrayIndex] + pos);

            exeBuffer[bufferIndex++] = CMD_JAE;
            exeBuffer[bufferIndex++] = arg;

        } else if (strcmp(command, "je") == 0) {
            int arg = get_jmp_args(array[arrayIndex] + pos);

            exeBuffer[bufferIndex++] = CMD_JE;
            exeBuffer[bufferIndex++] = arg;

        } else if (strcmp(command, "jne") == 0) {
            int arg = get_jmp_args(array[arrayIndex] + pos);

            exeBuffer[bufferIndex++] = CMD_JNE;
            exeBuffer[bufferIndex++] = arg;

        } else if (string_is_empty(array[arrayIndex])) {
            continue;

        } else {
            
        }
    }

    return exeBuffer;
}

compiler_errors get_exe_file(int* exeBuffer, const int sizeOfExeBuffer, const char* path) {
    if ((exeBuffer == NULL) || (path == NULL)) {
        PRINT_ERROR(NULLPTR);
        return NULLPTR;
    }

    FILE* exeFile = fopen(path, "wb"); 
    
    if (!file_is_open(exeFile)) {
        PRINT_ERROR(FILE_WASNT_OPEN);
        return FILE_WASNT_OPEN;
    }

    fwrite(signature, sizeof(int), sizeof(signature) / sizeof(signature[0]), exeFile);
    fwrite(exeBuffer, sizeof(int), sizeOfExeBuffer, exeFile);

    fclose(exeFile);

    return NOERRORS;
}
