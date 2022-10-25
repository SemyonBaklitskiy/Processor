#pragma once

typedef double elem_t;

enum commands {
    CMD_HLT  = 0,
    CMD_PUSH = 1,
    CMD_POP  = 2,
    CMD_ADD  = 3,
    CMD_SUB  = 4,
    CMD_MUL  = 5,
    CMD_DIV  = 6,
    CMD_OUT  = 7,
    CMD_DUP  = 9,
    CMD_JMP  = 10,
    CMD_JB   = 11,
    CMD_JBE  = 12,
    CMD_JA   = 13,
    CMD_JAE  = 14,
    CMD_JE   = 15,
    CMD_JNE  = 16, 
};

enum allErrors {
    NOERRORS               = 0,
    FILE_WASNT_OPEN_COMMON = 1,
    RETURNED_NULL_COMMON   = 2,
    NULLPTR_COMMON         = 3,
    SYNTAX_ERROR           = 4,
    HLT_NOT_FOUND          = 5,
    FEW_ARGS               = 6,
    MANY_ARGS              = 7,
    LABEL_ERROR            = 8,
    WRONG_ARGS             = 9,
    STACK_ERROR            = 10,
    WRONG_EXE_FILE         = 11,
    SEGMENTATION_FAULT     = 12,
    DEVISION_BY_ZERO       = 13,
    WRONG_RAM_ARGS         = 14,
    SOME_ERROR             = 15,
};

enum reg {
    REG_NULL = 0,
    REG_RAX  = 1,
    REG_RBX  = 2,
    REG_RCX  = 3,
    REG_RDX  = 4,
};

enum command_options {
    ARG_IMMED = 0X10,
    ARG_REG   = 0X20,
    ARG_RAM   = 0X40,
};

const int amountOfRegs = 5;
const int sizeOfRam = 1000;

char* get_name_stdin(const char* text);