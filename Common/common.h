#pragma once

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
    RETURNED_NULL_COMMON   = 2,    // 1 << 1
    NULLPTR_COMMON         = 4,
    SYNTAX_ERROR           = 8,
    HLT_NOT_FOUND          = 16,
    FEW_ARGS               = 32,
    MANY_ARGS              = 64,
    LABEL_ERROR            = 128,
    WRONG_ARGS             = 256,
    STACK_ERROR            = 512,
    WRONG_EXE_FILE         = 1024,
    SEGMENTATION_FAULT     = 2048,
    DEVISION_BY_ZERO       = 4096,
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

const int VERSION = 1; 
const int SIGNATURE[3] = { 'S', 'B', VERSION };
const int amountOfRegs = 5;
const int sizeOfRam = 1000;

char* get_name_stdin(const char* text);