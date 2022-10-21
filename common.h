enum commands {
    CMD_HLT = 0,
    CMD_PUSH = 1,
    CMD_POP = 2,
    CMD_ADD = 3,
    CMD_SUB = 4,
    CMD_MUL = 5,
    CMD_DIV = 6,
    CMD_OUT = 7,
    CMD_DUP = 9,
    CMD_JMP = 10,
    CMD_JB = 11,
    CMD_JBE = 12,
    CMD_JA = 13,
    CMD_JAE = 14,
    CMD_JE = 15,
    CMD_JNE = 16, 
};

enum reg {
    REG_NULL = 0,
    REG_RAX = 1,
    REG_RBX = 2,
    REG_RCX = 3,
    REG_RDX = 4,
};

enum command_options {
    ARG_IMMED = 0X10,
    ARG_REG = 0X20,
    ARG_RAM = 0X40,
};