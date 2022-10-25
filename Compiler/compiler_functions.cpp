#include <stdlib.h>
#include <string.h>
#include <cctype>
#include "../Common/common_functions.h"
#include "compiler_functions.h"

//TODO NO COPYPAST

static const int poison = -1;

static const unsigned int sizeOfLabels = 100;

static unsigned int currentLabel = 0;

static unsigned const int sizeOfCommands = 25;

static struct table_label labels[sizeOfLabels] = { {} }; 

#define compilation_errors(error, command, line) processor_of_errors(error, command, line, __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define INSERT_INT(data) *((int*)(exeBuffer + intDataCounter * sizeof(int) + elemtDataCounter * (sizeof(elem_t)))) = data; ++intDataCounter
#define INSERT_ELEM_T(data) *((elem_t*)(exeBuffer + intDataCounter * sizeof(int) + elemtDataCounter * (sizeof(elem_t)))) = argument; ++elemtDataCounter

static void fill_labels(const char* name);
static int amount_of_lines(char* buffer);
static bool string_is_empty(const char* str);
static int find_label_in_table(const char* labelName);
static int get_label(char* labelName);
static int get_jmp_args(const char* args);
static bool is_label(const char* str);
static int make_label(char* str, const long int ip);
static bool correct_forward_labels(char** array, const int sizeOfArray);
static int reg_number(const char* argument);
static int check_all_options(char* argument);
static bool correct_push_and_pop_argument(char* argument, long int* size);
static int get_reg_number_and_immed(char* argument, int* immed, int* regNumber);
static long int exe_buffer_size_and_check(char** array, const int sizeOfArray);

char* get_buffer(const char* path) {
    if (path == NULL) 
        PRINT_ERROR(NULLPTR_COMMON);
    

    FILE* file = fopen(path, "r");

    if (!file_exist(file)) {
        PRINT_ERROR(FILE_WASNT_OPEN_COMMON);
        return NULL;
    }

    long int size = get_file_size(file);

    char* buffer = (char*)calloc(size + 1, sizeof(char));

    if (buffer == NULL) {
        PRINT_ERROR(RETURNED_NULL_COMMON);
        return NULL;
    }

    fread(buffer, sizeof(char), size, file);
    buffer[size] = '\0';

    fclose(file);
    return buffer;
}

char** get_array_of_lines(char* buffer, int* amountOfLines) {
    if (buffer == NULL || amountOfLines == NULL) {
        PRINT_ERROR(NULLPTR_COMMON);
        return NULL;
    }

    int amount = amount_of_lines(buffer);

    *amountOfLines = amount;

    char** array = (char**)calloc(*amountOfLines, sizeof(char*));

    if (array == NULL) {
        PRINT_ERROR(RETURNED_NULL_COMMON);
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

char* get_exe_buffer(char** array, const int sizeOfArray, long int* sizeOfExeBuffer) {
    if ((array == NULL) || (sizeOfExeBuffer == NULL)) {
        PRINT_ERROR(NULLPTR_COMMON);
        return NULL;
    }

    long int size = exe_buffer_size_and_check(array, sizeOfArray);

    if (size == -1)
        return NULL;

    char* exeBuffer = (char*)calloc(size, sizeof(char));

    if (exeBuffer == NULL) {
        PRINT_ERROR(RETURNED_NULL_COMMON);
        return NULL;
    }

    *sizeOfExeBuffer = size;

    unsigned int intDataCounter = 0;
    unsigned int elemtDataCounter = 0; 

    for (int arrayIndex = 0; arrayIndex < sizeOfArray; ++arrayIndex) {
        char command[25] = "";
        int pos = 0;

        sscanf(array[arrayIndex], " %s %n ", command, &pos);

        if (strcmp(command, "push") == 0) {
            elem_t argument = 0.0;

            if (sscanf(array[arrayIndex] + pos, " %lf ", &argument) > 0) {
                INSERT_INT(CMD_PUSH | ARG_IMMED);
                INSERT_ELEM_T(argument);

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
                    INSERT_INT(options);
                    INSERT_INT(val);
                    INSERT_INT(immed);

                } else {
                    INSERT_INT(options);
                    INSERT_INT(val);
                }
           }

        } else if (strcmp(command, "pop") == 0) {
            if (string_is_empty(array[arrayIndex] + pos)) {
                INSERT_INT(CMD_POP | ARG_IMMED);

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
                    INSERT_INT(options);
                    INSERT_INT(val);
                    INSERT_INT(immed);
                    
                } else {
                    INSERT_INT(options);
                    INSERT_INT(val);
                }
            }

        } else if (strcmp(command, "add") == 0) {
            INSERT_INT(CMD_ADD);

        } else if (strcmp(command, "sub") == 0) {
            INSERT_INT(CMD_SUB);

        } else if (strcmp(command, "mul") == 0) {
            INSERT_INT(CMD_MUL);

        } else if (strcmp(command, "div") == 0) {
            INSERT_INT(CMD_DIV);

        } else if (strcmp(command, "out") == 0) {
            INSERT_INT(CMD_OUT);

        } else if (strcmp(command, "hlt") == 0) {
            INSERT_INT(CMD_HLT);

        } else if (strcmp(command, "dup") == 0) {
            INSERT_INT(CMD_DUP);

        } else if (strcmp(command, "jmp") == 0) {
            int arg = get_jmp_args(array[arrayIndex] + pos);

            INSERT_INT(CMD_JMP);
            INSERT_INT(arg); 

        } else if (strcmp(command, "jb") == 0) {
            int arg = get_jmp_args(array[arrayIndex] + pos);

            INSERT_INT(CMD_JB);
            INSERT_INT(arg);

        } else if (strcmp(command, "jbe") == 0) {
            int arg = get_jmp_args(array[arrayIndex] + pos);

            INSERT_INT(CMD_JBE);
            INSERT_INT(arg);

        } else if (strcmp(command, "ja") == 0) {
            int arg = get_jmp_args(array[arrayIndex] + pos);

            INSERT_INT(CMD_JA);
            INSERT_INT(arg);

        } else if (strcmp(command, "jae") == 0) {
            int arg = get_jmp_args(array[arrayIndex] + pos);

            INSERT_INT(CMD_JAE);
            INSERT_INT(arg);

        } else if (strcmp(command, "je") == 0) {
            int arg = get_jmp_args(array[arrayIndex] + pos);

            INSERT_INT(CMD_JE);
            INSERT_INT(arg);

        } else if (strcmp(command, "jne") == 0) {
            int arg = get_jmp_args(array[arrayIndex] + pos);

            INSERT_INT(CMD_JNE);
            INSERT_INT(arg);

        } else if (string_is_empty(array[arrayIndex])) {
            continue;

        } else {
            
        }
    }

    return exeBuffer;
}

allErrors get_exe_file(char* exeBuffer, const long int sizeOfExeBuffer, const char* path) {
    if ((exeBuffer == NULL) || (path == NULL)) {
        PRINT_ERROR(NULLPTR_COMMON);
        return NULLPTR_COMMON;
    }

    FILE* exeFile = fopen(path, "wb"); 
    
    if (!file_exist(exeFile)) {
        PRINT_ERROR(FILE_WASNT_OPEN_COMMON);
        return FILE_WASNT_OPEN_COMMON;
    }

    fwrite(SIGNATURE, sizeof(char), sizeof(SIGNATURE), exeFile);
    fwrite(exeBuffer, sizeof(char), sizeOfExeBuffer, exeFile);

    fclose(exeFile);

    return NOERRORS;
}

static void fill_labels(const char* name) {
    for (unsigned int labelsIndex = 0; labelsIndex < sizeOfLabels; ++labelsIndex) {
        labels[labelsIndex].ip = poison;
        labels[labelsIndex].name = name;
    }
}

static int amount_of_lines(char* buffer) {
    int amount = 0; 

    for (unsigned int bufferPointer = 0; buffer[bufferPointer] != '\0'; ++bufferPointer) {
        if (buffer[bufferPointer] == '\n') {
            ++amount;
        }
    }

    return amount + 1;
}

static bool string_is_empty(const char* str) {
    for (int strIndex = 0; str[strIndex] != '\0'; ++strIndex) {
        if (!isspace(str[strIndex]))
            return false; 
    }

    return true;
}

static int find_label_in_table(const char* labelName) {
    for (unsigned int labelsIndex = 0; labelsIndex < sizeOfLabels; ++labelsIndex) {
        if (strcmp(labelName, labels[labelsIndex].name) == 0) 
            return labels[labelsIndex].ip;     
    }

    return -1;
}

static int get_label(char* labelName) {
    if (labelName[0] == ':')
        ++labelName;

    unsigned int numberOfLabel = 0;

    int pos = 0;

    int check = sscanf(labelName, "%d %n", &numberOfLabel, &pos);

    if (check <= 0) {
        char nameOfLabel[sizeOfCommands] = "";
        sscanf(labelName, "%s %n", nameOfLabel, &pos);

        if (string_is_empty(labelName + pos)) {
            return find_label_in_table(nameOfLabel);

        } else {
            return -1;
        }

    } else {
        if ((numberOfLabel < sizeOfLabels) && string_is_empty(labelName + pos)) {
            return labels[numberOfLabel].ip;

        } else {
            return -1;
        }
    }
    
}

static int get_jmp_args(const char* args) {
    unsigned int numberOfLabel = 0;
    int pos = 0;

    if (string_is_empty(args)) 
        return -2;

    if (sscanf(args, " %d %n", &numberOfLabel, &pos) <= 0) {
        char labelName[sizeOfCommands] = "";
        sscanf(args, " %s ", labelName);

        return get_label(labelName);

    } else {
        if ((numberOfLabel < sizeOfLabels) && (string_is_empty(args + pos)))
            return labels[numberOfLabel].ip;
        
        return -1;
    }
}

static bool is_label(const char* str) {
    return !((str[strlen(str) - 1] != ':') || (strlen(str) <= 1));
}

static int make_label(char* str, const long int ip) {
    str[strlen(str) - 1] = '\0';

    while (isspace(*str))
        ++str;

    int numberOfLabel = 0;

    int check = sscanf(str, "%d", &numberOfLabel);

    if (check > 0) {
        if ((numberOfLabel < 0) || (labels[numberOfLabel].ip != poison)) 
            return -1;

        labels[numberOfLabel].ip = ip;

    } else {
        if (find_label_in_table(str) != poison)
            return -1;

        for (currentLabel = 0; currentLabel < sizeOfLabels; ++currentLabel) {
            if (labels[currentLabel].ip == poison)
                break;
        }

        if ((currentLabel == sizeOfLabels - 1) && (labels[currentLabel].ip != poison))
            return -1;

        labels[currentLabel].name = str;
        labels[currentLabel].ip = ip;
        ++currentLabel;
    }

    return 0;
}

static bool correct_forward_labels(char** array, const int sizeOfArray) {
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

static int check_all_options(char* argument) {
    char* posOfPlus = strchr(argument, '+');

    if ((posOfPlus == NULL) || (posOfPlus == argument)) 
        return 0;  

    char str2[sizeOfCommands] = "";
    char str1[sizeOfCommands] = "";

    int pos = 0;

    sscanf(posOfPlus + 1, " %s %n", str2, &pos); 
    posOfPlus[0] = '\0';
    sscanf(argument, " %s ", str1);

    if (!string_is_empty(posOfPlus + 1 + pos)) 
        return -1;

    int number = 0;
    int nextPos = 0;

    int isNumberStr1 = sscanf(str1, " %d %n", &number, &nextPos);
    int isNumberStr2 = sscanf(str2, " %d %n", &number, &nextPos);

    if ((isNumberStr1 > 0) && (isNumberStr2 > 0)) {
        return -1;

    } else if ((isNumberStr1 <= 0) && (isNumberStr2 <= 0)) {
        return -1;

    } else if ((isNumberStr1 > 0) && (string_is_empty(str1 + nextPos))) {
        int regNumber = reg_number(str2);

        if (regNumber == -1)
            return -1;

    } else if ((isNumberStr2 > 0) && (string_is_empty(str2 + nextPos))) {
        int regNumber = reg_number(str1);

        if (regNumber == -1)
            return -1;

    } else {
        return -1;
    }

    posOfPlus[0] = '+';

    return 1;
}

static bool correct_push_and_pop_argument(char* argument, long int* size) { 
    if ((argument[0] == '[') && (argument[strlen(argument) - 1] == ']')) {
        argument[strlen(argument) - 1] = '\0';
        ++argument;

        int allOptions = check_all_options(argument);

        if (allOptions == 1) {
            *(size) += 2 * sizeOfInt;
            return true;

        } else if (allOptions == -1) {
            return false;
        }
    }

    *size += sizeOfInt;

    int arg = 0;
    int pos = 0;

    int isNumber = sscanf(argument, " %d %n ", &arg, &pos);

    if ((isNumber > 0) && (arg >= 0) && (arg < sizeOfRam) && (string_is_empty(argument + pos))) 
        return true;
    
    if (reg_number(argument) == -1)
        return false;

    return true;
}

static int get_reg_number_and_immed(char* argument, int* immed, int* regNumber) {
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

static long int exe_buffer_size_and_check(char** array, const int sizeOfArray) {
    fill_labels("Noname");

    long int size = 0;
    bool hltFound = false;
    bool errorFound = false;
    bool hasForwardLabels = false;

    for (int arrayIndex = 0; arrayIndex < sizeOfArray; ++arrayIndex) {
        char command[sizeOfCommands] = "";
        int pos = 0;

        sscanf(array[arrayIndex], " %s %n ", command, &pos);

        if (strcmp(command, "push") == 0) {
            size += sizeOfInt; 
            elem_t argument = 0.0;

            int nextPos = 0;
            int checkForArgs = sscanf(array[arrayIndex] + pos, " %lf %n", &argument, &nextPos);

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

            } else {
                size += sizeOfElemt;
            }

        } else if (strcmp(command, "pop") == 0) {
            elem_t argument = 0.0;
            size += sizeOfInt;

            int nextPos = 0;
            int checkForArgs = sscanf(array[arrayIndex] + pos, " %lf %n", &argument, &nextPos);

            if (string_is_empty(array[arrayIndex] + pos)) {
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
                    continue;
                }
                
            }   else {
                compilation_errors(WRONG_ARGS, command, arrayIndex + 1);
                errorFound = true;
            }

        } else if (strcmp(command, "add") == 0) {
            size += sizeOfInt;

            if (!string_is_empty(array[arrayIndex] + pos)) {
                compilation_errors(MANY_ARGS, command, arrayIndex + 1);
                errorFound = true;
            }

        } else if (strcmp(command, "sub") == 0) {
            size += sizeOfInt;

            if (!string_is_empty(array[arrayIndex] + pos)) {
                compilation_errors(MANY_ARGS, command, arrayIndex + 1);
                errorFound = true;
            }

        } else if (strcmp(command, "mul") == 0) {
            size += sizeOfInt;

            if (!string_is_empty(array[arrayIndex] + pos)) {
                compilation_errors(MANY_ARGS, command, arrayIndex + 1);
                errorFound = true;
            }

        } else if (strcmp(command, "div") == 0) {
            size += sizeOfInt;

            if (!string_is_empty(array[arrayIndex] + pos)) {
                compilation_errors(MANY_ARGS, command, arrayIndex + 1);
                errorFound = true;
            }

        } else if (strcmp(command, "out") == 0) {
            size += sizeOfInt;

            if (!string_is_empty(array[arrayIndex] + pos)) { 
                compilation_errors(MANY_ARGS, command, arrayIndex + 1);
                errorFound = true;
            }

        } else if (strcmp(command, "dup") == 0) {
            size += sizeOfInt;

            if (!string_is_empty(array[arrayIndex] + pos)) { 
                compilation_errors(MANY_ARGS, command, arrayIndex + 1);
                errorFound = true;
            }

        } else if ((strcmp(command, "jmp") == 0) || (strcmp(command, "jb") == 0) || (strcmp(command, "jbe") == 0) || (strcmp(command, "ja") == 0) 
                || (strcmp(command, "jae") == 0) || (strcmp(command, "je") == 0) || (strcmp(command, "jne") == 0)) {
            size += 2 * sizeOfInt;

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

        } else if (strcmp(command, "hlt") == 0) {
            size += sizeOfInt;
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
