#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define DOUBLE

#ifdef INT
    typedef int elem_t;

#elif defined DOUBLE
    typedef double elem_t;

#else 
    typedef example elem_t;
#endif

/// enum type errors
enum stack_errors { // TODO: <<
    NO_IMPORTANT_ERRORS = 1, ///< If no errors happened
    OVERFLOW = 2, ///< If there was stack overflow (realloc returned NULL in push)
    UNDERFLOW = 4, ///< If there was stack underflow (pop when size == 0) 
    RETURNED_NULL = 8, ///< If realloc returned NULL in pop
    WRONG_PARAMETERS = 16, ///< If wrong parameters were given to stack_constructor (stack_distructor)
    NULLPTR = 32, ///< If in any function (except c-tor, dis-tor, dump) were given NULL as a parameter
    FILE_WASNT_OPEN = 64, ///< If any file wasn`t open
    MISMACH_STRUCT_CANARY = 128, ///< If canary in structure was broken
    MISMATCH_BUFFER_CANARY = 256, ///< If canary in buffer was broken
    MISMATCH_HASH = 512, ///< If there was mismatch between current hash and st.hash field
    HAS_BEEN_DESTRUCTED = 1024,
    WASNT_CREATED = 2048,
    HAS_BEEN_CREATED = 4096,
};

struct stack {
#ifdef CANARY_PROT
    unsigned long long int leftCanary;
#endif
#ifdef HASH_PROT
    unsigned int hash;
#endif
    bool created = false;
    bool destroyed = false;
    elem_t* buffer;
    unsigned int size;
    unsigned int capacity; 
    const char* file;
    const char* function;
    int line;
    const char* name;
    long unsigned int error;
#ifdef CANARY_PROT
    unsigned long long rightCanary;
#endif
};

stack_errors stack_constructor(struct stack* st, const int cp, const char* name, const char* file, const char* function, const int line);

stack_errors stack_destructor(struct stack* st);

stack_errors stack_pop(struct stack* st, elem_t* element);

stack_errors stack_push(struct stack* st, const elem_t element);

#ifdef DEBUG
void debug(struct stack* st);
#endif

#define VARNAME(var) #var + (#var[0] == '&')

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define dump stack_dump(st, __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define my_assert_if(condition, error, arg) if (condition) {processor_of_errors(error, __FILE__, __PRETTY_FUNCTION__, __LINE__); return arg} 

static void print_elements(int* buffer, FILE* stream, unsigned int capacity);
static void print_elements(double* buffer, FILE* stream, unsigned int capacity);
static void print_elements(elem_t* buffer, FILE* stream, unsigned int capacity);

static const long unsigned int poison = 0XDEADDEAD;

static const char* logFile = "Output_files/log.txt"; 

#ifdef DEBUG
const char* debugFile = "Output_files/debug.txt";
#endif

#ifdef CANARY_PROT
    static const unsigned long long int canaryDefinition = 0XBAADF00D;
#endif

#ifdef HASH_PROT
    static unsigned int HashRot13(elem_t* buffer, const unsigned int bytes) {

#ifdef CANARY_PROT
    char* begin = ((char*)buffer + sizeof(canaryDefinition));
#else 
    char* begin = (char*)buffer;
#endif
    unsigned int hash = 0;

    for(unsigned int i = 0; i < bytes; ++i) {
        hash += (unsigned char)(*(begin + i));
        hash -= (hash << 13) | (hash >> 19);
    }

    return hash;
}
#endif

static void processor_of_errors(stack_errors error, const char* fileName, const char* functionName, const int line) { 
    if ((fileName == NULL) || (functionName == NULL))
        return;

    switch (error) {

    case FILE_WASNT_OPEN:
        printf("In file %s function %s on line %d: file wasn`t open\n", fileName, functionName, line);
        break;

    case NULLPTR:
        printf("In file %s function %s on line %d: NULL was given as a parameter\n", fileName, functionName, line);
        break;

    default:
        return;
    }   
}

static bool file_is_open(FILE* stream) {
    if (stream == NULL) 
        return false;

    return true;   
}

static void print_elements(int* buffer, FILE* stream, unsigned int capacity) {
    my_assert_if(stream == NULL, NULLPTR, ;)

#ifdef CANARY_PROT
    for (unsigned int i = 0; i < capacity; ++i) 
        fprintf(stream, "data[%d] = %d\n", i, *((int*)((char*)buffer + sizeof(canaryDefinition) + i * sizeof(int))));
#else 
    for (unsigned int i = 0; i < capacity; ++i) 
        fprintf(stream, "data[%d] = %d\n", i, buffer[i]);
#endif
}

static void print_elements(double* buffer, FILE* stream, unsigned int capacity) {
    my_assert_if(stream == NULL, NULLPTR, ;)

#ifdef CANARY_PROT
    for (unsigned int i = 0; i < capacity; ++i) 
        fprintf(stream, "data[%d] = %lf\n", i, *((double*)((char*)buffer + sizeof(canaryDefinition) + i * sizeof(double))));
#else 
    for (unsigned int i = 0; i < capacity; ++i) 
        fprintf(stream, "data[%d] = %lf\n", i, buffer[i]);
#endif
}

//static void print_elements(elem_t* buffer, FILE* stream, unsigned int capacity) {
    /* Write your rules to print elements here*/
//} 

static void print_errors(struct stack* st, FILE* stream) {
    my_assert_if((st == NULL) || (stream == NULL), NULLPTR, ;)

    fprintf(stream, "Errors:\n");

    if (st->error & OVERFLOW)
        fprintf(stream, "   Stack overflow\n");
    if (st->error & UNDERFLOW)
        fprintf(stream, "   Stack underflow\n");
    if (st->error & RETURNED_NULL)
        fprintf(stream, "   Stack couldn`t change capacity, calloc returned NULL\n");
    if (st->error & WRONG_PARAMETERS)
        fprintf(stream, "   Were given wrong parameters to function stack_constructor\n");
    if (st->error & MISMACH_STRUCT_CANARY)
        fprintf(stream, "   Struct canary was broken\n");
    if (st->error & MISMATCH_BUFFER_CANARY)
        fprintf(stream, "   Buffer canary was broken\n");
    if (st->error & MISMATCH_HASH)
        fprintf(stream, "   Mismatch between hashes\n");
    if (st->error & HAS_BEEN_DESTRUCTED) 
        fprintf(stream, "   Trying to destruct stack that has already been destroyed\n");
    if (st->error & WASNT_CREATED) 
        fprintf(stream, "   Trying to use stack that wasn`t created\n");
    if (st->error & HAS_BEEN_CREATED) 
        fprintf(stream, "   Trying to create stack that has already been created\n");
}

static void info_in_logfile(struct stack* st, FILE* stream) {
    my_assert_if(st == NULL || stream == NULL, NULLPTR, ;) 

    fprintf(stream, "Stack \"%s\" created in file %s in function %s on line %d\n", st->name, st->file, st->function, st->line);
    fprintf(stream, "Size = %d\n", st->size);
    fprintf(stream, "Capacity = %d\n", st->capacity);
    fprintf(stream, "Pointer to buffer = %p\n", st->buffer);

#ifdef CANARY_PROT
    fprintf(stream, "Left canary in structure = %llX\n", st->leftCanary);
    fprintf(stream, "Right canary in structure = %llX\n", st->rightCanary);
#endif

#ifdef HASH_PROT
    fprintf(stream, "Hash = %d\n", st->hash);
#endif
    print_errors(st, stream);

    if (st->buffer != NULL) {
    #ifdef CANARY_PROT
        fprintf(stream, "Left canary in buffer = %llX\n", *((unsigned long long int*)st->buffer)); 
        fprintf(stream, "Right canary in buffer = %llX\n", *((unsigned long long int*)((char*)st->buffer + sizeof(canaryDefinition) + sizeof(elem_t) * st->capacity)));

    #endif
        print_elements(st->buffer, stream, st->capacity); 
    }
}

static void stack_dump(struct stack* st, const char* file, const char* function, const int line) {
    my_assert_if((st == NULL) || (file == NULL) || (function == NULL), NULLPTR, ;) 

    FILE* stream = fopen(logFile, "w");

    my_assert_if(!file_is_open(stream), FILE_WASNT_OPEN, ;)

    fprintf(stream, "In file %s in function %s on line %d stack_dump called\n", file, function, line);
    info_in_logfile(st, stream);

    fclose(stream);
    printf("Look at the output_files/log.txt\n");   
}

#if defined CANARY_PROT || defined HASH_PROT

static stack_errors verification(struct stack* st) {
    my_assert_if(st == NULL, NULLPTR, NULLPTR;) 
    bool errorFound = false;

#ifdef CANARY_PROT
    unsigned long long int leftCanary = st->leftCanary;
    unsigned long long int rightCanary = st->rightCanary;

    if ((leftCanary != canaryDefinition) || (rightCanary != canaryDefinition)) {
        st->error |= MISMACH_STRUCT_CANARY;
        errorFound = true;
    }
    
    if (((*((unsigned long long int*)st->buffer)) != canaryDefinition) || 
        ((*((unsigned long long int*)((char*)st->buffer + sizeof(canaryDefinition) + st->capacity * sizeof(elem_t)))) != canaryDefinition)) {
            st->error |= MISMATCH_BUFFER_CANARY;
            errorFound = true;
    }
            
#endif

#ifdef HASH_PROT
    if (HashRot13(st->buffer, st->capacity * sizeof(elem_t)) != st->hash) {
        st->error |= MISMATCH_HASH;
        errorFound = true;
    }
#endif

    if (errorFound) {
        dump;
        return MISMACH_STRUCT_CANARY;

    } else {
        return NO_IMPORTANT_ERRORS;
    }
}

#endif

static stack_errors resize(struct stack* st, const unsigned int oldCapacity, const unsigned int newCapacity) {
    my_assert_if(st == NULL, NULLPTR, NULLPTR;) 

#ifdef CANARY_PROT
    st->buffer = (elem_t*)realloc(st->buffer, newCapacity * sizeof(elem_t) + 2 * sizeof(canaryDefinition));    
    
    if (st->buffer == NULL) 
        return RETURNED_NULL;

    *((unsigned long long int*)st->buffer) = canaryDefinition; 
    *((unsigned long long int*)((char*)st->buffer + sizeof(canaryDefinition) + newCapacity * sizeof(elem_t))) = canaryDefinition;

    if (newCapacity > oldCapacity)
        memset((char*)st->buffer + sizeof(canaryDefinition) + oldCapacity * sizeof(elem_t), poison, (newCapacity - oldCapacity) * sizeof(elem_t));
        

    
#endif

#ifndef CANARY_PROT
    st->buffer = (elem_t*)realloc(st->buffer, newCapacity * sizeof(elem_t));

    if (st->buffer == NULL) 
       return RETURNED_NULL;

    if (newCapacity > oldCapacity)
        memset((char*)st->buffer + oldCapacity * sizeof(elem_t), poison, (newCapacity - oldCapacity) * sizeof(elem_t)); 
    

#endif 
    return NO_IMPORTANT_ERRORS;
}

stack_errors stack_constructor(struct stack* st, const int cp, const char* name, const char* file, const char* function, const int line) {
    my_assert_if(st == NULL, NULLPTR, NULLPTR;) 

    if (st->created) {
        st->error |= HAS_BEEN_CREATED;
        dump;
        return NO_IMPORTANT_ERRORS;
    }

    if ((cp <= 0) || (file == NULL) || (function == NULL) || (name == NULL)) {
        st->error = 0;
        st->error |= WRONG_PARAMETERS;
        st->buffer = NULL;
        st->size = 0;
        st->name = name;
        st->capacity = 0;
        st->file = file;
        st->function = function;
        st->line = line;
        dump;
        return WRONG_PARAMETERS;
    }

    unsigned int capacity = (unsigned int)cp;

    st->capacity = capacity;
    st->size = 0; 
    st->name = name;
    st->file = file;
    st->function = function;
    st->line = line;
    st->error = 0;
    st->created = true;

#ifdef CANARY_PROT
    st->rightCanary = canaryDefinition;
    st->leftCanary = canaryDefinition;

    st->buffer = (elem_t*)calloc(capacity * sizeof(elem_t) + 2 * (sizeof(canaryDefinition)), sizeof(char));

    if (st->buffer == NULL) {
        st->error |= RETURNED_NULL;
        dump;
        return RETURNED_NULL;
    }   

    *((unsigned long long int*)st->buffer) = canaryDefinition; 
    *((unsigned long long int*)((char*)st->buffer + sizeof(canaryDefinition) + capacity * sizeof(elem_t))) = canaryDefinition;
    
    memset((char*)st->buffer + sizeof(canaryDefinition), poison, capacity * sizeof(elem_t));

#endif

#ifndef CANARY_PROT
    st->buffer = (elem_t*)calloc(capacity, sizeof(elem_t));

    if (st->buffer == NULL) {
        st->error |= RETURNED_NULL;
        dump;
        return RETURNED_NULL;
    }

    memset(st->buffer, poison, capacity * sizeof(elem_t));

#endif

#ifdef HASH_PROT
    st->hash = HashRot13(st->buffer, capacity * sizeof(elem_t));
#endif

#if defined HASH_PROT || defined CANARY_PROT
    return verification(st);
#else
    return NO_IMPORTANT_ERRORS;
#endif
}

static void stack_not_created(struct stack* st) {
    my_assert_if(st == NULL, NULLPTR, ;)

    printf("Look at the output_files/log.txt\n");

    FILE* stream = fopen(logFile, "w");
    my_assert_if(!file_is_open(stream), FILE_WASNT_OPEN, ;)
    fprintf(stream, "Stack at adress %p wasn`t created\n", st);
    fclose(stream);
}

stack_errors stack_push(struct stack* st, const elem_t element) {
    my_assert_if(st == NULL, NULLPTR, NULLPTR;) 

    if (!st->created) {
        stack_not_created(st);
        return WASNT_CREATED;
    }

    if (st->destroyed) {
        st->error |= HAS_BEEN_DESTRUCTED;
        dump;
        return HAS_BEEN_DESTRUCTED;
    }

#if defined HASH_PROT || defined CANARY_PROT
    stack_errors error = verification(st);
    if (error != NO_IMPORTANT_ERRORS)
        return error;
#endif

    unsigned int size = st->size;
    unsigned int capacity = st->capacity;
    
    if (size >= capacity) {
        stack_errors errorInResize = resize(st, capacity, capacity * 2);

        if (errorInResize == NULLPTR) { 
            return NULLPTR;

        }   else if (errorInResize == RETURNED_NULL) {
            st->error |= OVERFLOW;
            dump;
            return OVERFLOW;
        }

        ++size;
        st->size = size;

        capacity *= 2;
        st->capacity = capacity;

    #ifdef CANARY_PROT 
        *((elem_t*)((char*)st->buffer + sizeof(canaryDefinition) + (size - 1) * sizeof(elem_t))) = element; 
        
    #endif

    #ifndef CANARY_PROT
        st->buffer[size - 1] = element;
    #endif

    #ifdef HASH_PROT
        st->hash = HashRot13(st->buffer, capacity * sizeof(elem_t));
    #endif

    #if defined HASH_PROT || defined CANARY_PROT
        return verification(st);   
    #else 
        return NO_IMPORTANT_ERRORS;
    #endif
    }

    ++size;
    st->size = size;

#ifdef CANARY_PROT  
    *((elem_t*)((char*)st->buffer + sizeof(canaryDefinition) + (size - 1) * sizeof(elem_t))) = element; 
        
#endif

#ifndef CANARY_PROT
    st->buffer[size - 1] = element;
#endif

#ifdef HASH_PROT
    st->hash = HashRot13(st->buffer, capacity * sizeof(elem_t));
#endif

#if defined HASH_PROT || defined CANARY_PROT
    return verification(st);
#else 
    return NO_IMPORTANT_ERRORS;
#endif
}

stack_errors stack_pop(struct stack* st, elem_t* element) {
    my_assert_if(st == NULL, NULLPTR, NULLPTR;) 

    if (!st->created) {
        stack_not_created(st);
        return WASNT_CREATED;
    }

    if (st->destroyed) {
        st->error |= HAS_BEEN_DESTRUCTED;
        dump;
        return HAS_BEEN_DESTRUCTED;
    }

#if defined HASH_PROT || defined CANARY_PROT
    stack_errors error = verification(st);
    if (error != NO_IMPORTANT_ERRORS)
        return error;
#endif

    unsigned int size = st->size;
    unsigned int capacity = st->capacity;

    if (size == 0) {
        st->error |= UNDERFLOW;
        dump; 
        return UNDERFLOW;
    }

#ifdef CANARY_PROT
    *element = *((elem_t*)((char*)st->buffer + sizeof(canaryDefinition) + (size - 1) * sizeof(elem_t)));
    memset((char*)st->buffer + sizeof(canaryDefinition) + (size - 1) * sizeof(elem_t), poison, sizeof(elem_t));
#endif

#ifndef CANARY_PROT
    *element = st->buffer[size - 1];
    memset((char*)st->buffer + (size - 1) * sizeof(elem_t), poison, sizeof(elem_t));

#endif
    --size;
    st->size = size;
    
    if (((capacity / 2) >= size) && (capacity > 1)) {
        stack_errors errorInResize = resize(st, capacity, capacity / 2); 

        if (errorInResize == NULLPTR) {
            return NULLPTR;

        } else if (errorInResize == RETURNED_NULL) {
            st->error |= RETURNED_NULL;
            dump;
            return RETURNED_NULL;
        }

        st->capacity = capacity / 2;
    }

#ifdef HASH_PROT
    st->hash = HashRot13(st->buffer, st->capacity * sizeof(elem_t));
#endif

#if defined HASH_PROT || defined CANARY_PROT
    return verification(st);
#else 
    return NO_IMPORTANT_ERRORS;
#endif
}

stack_errors stack_destructor(struct stack* st) {
    my_assert_if(st == NULL, NULLPTR, NULLPTR;)

    if (!st->created) {
        stack_not_created(st);
        return NO_IMPORTANT_ERRORS; 
    }

    if (st->destroyed) {
        st->error |= HAS_BEEN_DESTRUCTED;
        dump;
        return NO_IMPORTANT_ERRORS;
    }

#if defined HASH_PROT || defined CANARY_PROT
    stack_errors error = verification(st);
    if (error != NO_IMPORTANT_ERRORS)
        return error;
#endif

    st->size = 0;
    st->capacity = 0;
    free(st->buffer);
    st->buffer = NULL;
    st->destroyed = true;

#ifdef HASH_PROT
    st->hash = 0;
#endif

#ifdef CANARY_PROT
    st->leftCanary = 0;
    st->rightCanary = 0;
#endif

    st->error = 0;

    return NO_IMPORTANT_ERRORS;
}

#ifdef DEBUG

void debug(struct stack* st) {
    if (st == NULL) {
        my_assert(NULLPTR);
        return;
    }

    FILE* stream = fopen(debugFile, "a");

    if (st == NULL) {
        my_assert(NULLPTR);
        return;
    }

    fprintf(stream, "Stack \"%s\" created in file %s in function %s on line %d\n", st->name, st->file, st->function, st->line);
    fprintf(stream, "Size = %d\n", st->size);
    fprintf(stream, "Capacity = %d\n", st->capacity);
    fprintf(stream, "Pointer to buffer = %p\n", st->buffer);
#ifdef HASH_PROT
    fprintf(stream, "Hash = %d\n", st->hash);
#endif
    fprintf(stream, "Error = %d\n", st->error);

#ifdef CANARY_PROT
    fprintf(stream, "Left canary in structure = %llX\n", st->leftCanary);
    fprintf(stream, "Right canary in structure = %llX\n", st->rightCanary);

    fprintf(stream, "Left canary in buffer = %llX\n", *((unsigned long long int*)st->buffer)); 
    fprintf(stream, "Right canary in buffer = %llX\n", *((unsigned long long int*)((char*)st->buffer + sizeof(canaryDefinition) + sizeof(elem_t) * st->capacity)));
    

#if defined INT || defined DOUBLE
    if (st->buffer == NULL) 
        return;

    for (unsigned int i = sizeof(canaryDefinition) / sizeof(elem_t); i < st->capacity + sizeof(canaryDefinition) / sizeof(elem_t); ++i) 
        fprintf(stream, TEXT, i - sizeof(canaryDefinition) / sizeof(elem_t), st->buffer[i]);
    
#endif

#endif

#ifndef CANARY_PROT

#if defined INT || defined DOUBLE
    if (st->buffer == NULL) 
        return;

    for (unsigned int i = 0; i < st->capacity; ++i) 
        fprintf(stream, TEXT, i, st->buffer[i]);
#endif

#endif

    fclose(stream);  
}

#endif

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

bool file_exist(FILE* stream);

long unsigned int get_file_size(FILE* stream);

void processor_of_errors(allErrors error, const char* command, const int fileLine, const char* function, const char* name, const int line);

#define PRINT_ERROR(error) processor_of_errors(error, NULL, 0, __FILE__, __PRETTY_FUNCTION__, __LINE__)

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

#ifdef COMPILER

char* get_buffer(const char* path) {
    if (path == NULL) 
        PRINT_ERROR(NULLPTR_COMMON);
    

    FILE* file = fopen(path, "r");

    if (!file_exist(file)) {
        PRINT_ERROR(FILE_WASNT_OPEN_COMMON);
        return NULL;
    }

    long unsigned int size = get_file_size(file);

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

#endif

#ifdef CPU

char* get_buffer(const char* path, long unsigned int* sizeOfBuffer) { //check type of args
    if ((path == NULL) || (sizeOfBuffer == NULL)) 
        PRINT_ERROR(NULLPTR_COMMON);

    FILE* file = fopen(path, "rb");

    if (!file_exist(file)) {
        PRINT_ERROR(FILE_WASNT_OPEN_COMMON);
        return NULL;
    }

    long unsigned int size = get_file_size(file);

    if (size < sizeof(SIGNATURE)) {
        PRINT_ERROR(WRONG_EXE_FILE);
        return NULL;
    }

    if (!correct_signature(file)) 
        return NULL;

    char* buffer = (char*)calloc(size - sizeof(SIGNATURE), sizeof(char)); 

    if (buffer == NULL) {
        PRINT_ERROR(RETURNED_NULL_COMMON);
        return NULL;
    }

    fread(buffer, sizeof(char), size - sizeof(SIGNATURE), file);
    *sizeOfBuffer = size - sizeof(SIGNATURE);

    fclose(file);
    return buffer;
}

#endif

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
        printf("Error(s) in stack happened. Look at the log.txt\n");
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

    default:
        return;
    }
}

struct my_cpu {
    elem_t regs[amountOfRegs];
    struct stack st;
    elem_t ram[sizeOfRam];

    const char* fileName;
    const char* functionName;
    unsigned int line;
};

char* get_buffer(const char* path, long unsigned int* sizeOfBuffer); 

allErrors run(char* instructionsBuffer, const long unsigned int size);

#define stack_destor(pointer) if (stack_destructor(pointer) != NO_IMPORTANT_ERRORS) { PRINT_ERROR(STACK_ERROR); return STACK_ERROR; }
#define stack_ctor(pointer, capacity) if (stack_constructor(pointer, capacity, VARNAME(pointer),__FILE__, __PRETTY_FUNCTION__, __LINE__) != NO_IMPORTANT_ERRORS) { PRINT_ERROR(STACK_ERROR); return NULL; } 
#define push(pointer, element) if (stack_push(pointer, element) != NO_IMPORTANT_ERRORS) { PRINT_ERROR(STACK_ERROR); return STACK_ERROR; } 
#define pop(pointer, element) if (stack_pop(pointer, element) != NO_IMPORTANT_ERRORS) { PRINT_ERROR(STACK_ERROR); return STACK_ERROR; }
#define COMMAND_PUSH (instructionsBuffer[ip] == (CMD_PUSH | ARG_IMMED)) || (instructionsBuffer[ip] == (CMD_PUSH | ARG_REG)) || (instructionsBuffer[ip] == (CMD_PUSH | ARG_REG | ARG_RAM)) || (instructionsBuffer[ip] == (CMD_PUSH | ARG_RAM | ARG_IMMED)) || (instructionsBuffer[ip] == (CMD_PUSH | ARG_RAM | ARG_REG | ARG_IMMED))
#define COMMAND_POP (instructionsBuffer[ip] == (CMD_POP | ARG_IMMED)) || (instructionsBuffer[ip] == (CMD_POP | ARG_REG)) || (instructionsBuffer[ip] == (CMD_POP | ARG_REG | ARG_RAM)) || (instructionsBuffer[ip] == (CMD_POP | ARG_RAM | ARG_IMMED)) || (instructionsBuffer[ip] == (CMD_POP | ARG_RAM | ARG_REG | ARG_IMMED))
#define INT_INSTRUCTIONS_BUFFER(ip) *((int*)(instructionsBuffer + ip)) 
#define ELEM_T_INSTRUCTIONS_BUFFER(ip) *((elem_t*)(instructionsBuffer + ip)) 

static bool correct_signature(FILE* file);
static struct my_cpu* cpu_constructor(const char* fileName, const char* functionName, unsigned int line);

char* get_buffer(const char* path, long unsigned int* sizeOfBuffer) { 
    if (path == NULL) 
        PRINT_ERROR(NULLPTR_COMMON);

    FILE* file = fopen(path, "rb");

    if (!file_exist(file)) {
        PRINT_ERROR(FILE_WASNT_OPEN_COMMON);
        return NULL;
    }

    long unsigned int size = get_file_size(file);

    if (size < sizeof(SIGNATURE)) {
        PRINT_ERROR(WRONG_EXE_FILE);
        return NULL;
    }

    if (!correct_signature(file)) 
        return NULL;

    char* buffer = (char*)calloc(size - sizeof(SIGNATURE), sizeof(char)); 

    if (buffer == NULL) {
        PRINT_ERROR(RETURNED_NULL_COMMON);
        return NULL;
    }

    fread(buffer, sizeof(char), size - sizeof(SIGNATURE), file);
    *sizeOfBuffer = size - sizeof(SIGNATURE);

    fclose(file);
    return buffer;
}

allErrors run(char* instructionsBuffer, const long unsigned int size) {
    if (instructionsBuffer == NULL) {
        PRINT_ERROR(NULLPTR_COMMON);
        return NULLPTR_COMMON;
    }

    long unsigned int ip = 0;

    struct my_cpu* cpu = cpu_constructor(__FILE__, __PRETTY_FUNCTION__, __LINE__);
    
    if (cpu == NULL) 
        return NULLPTR_COMMON;

    while ((INT_INSTRUCTIONS_BUFFER(ip) != CMD_HLT) && (ip < size)) {
        if (COMMAND_PUSH) {
            if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_PUSH | ARG_IMMED | ARG_RAM)) {
                if ((INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands)) >= sizeOfRam) || (INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands)) < 0)) {
                    PRINT_ERROR(SEGMENTATION_FAULT);
                    return SEGMENTATION_FAULT;
                }

                push(&cpu->st, cpu->ram[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))]);
                ip += 2 * sizeof(commands);

            } else if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_PUSH | ARG_IMMED)) {
                push(&cpu->st, ELEM_T_INSTRUCTIONS_BUFFER(ip + sizeof(commands)));
                ip += sizeof(commands) + sizeof(elem_t);

            } else if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_PUSH | ARG_REG | ARG_RAM)) {
                if ((cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))] >= sizeOfRam) || (cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))] < 0)) {
                    PRINT_ERROR(SEGMENTATION_FAULT);
                    return SEGMENTATION_FAULT;

                } else if ((int)cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))] != cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))]) {
                    PRINT_ERROR(WRONG_ARGS);
                    return WRONG_ARGS;
                }

                push(&cpu->st, cpu->ram[(int)cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))]]);
                ip += 2 * sizeof(commands);

            } else if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_PUSH | ARG_REG)) {
                push(&cpu->st, cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))]);
                ip += 2 * sizeof(commands);

            } else if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_PUSH | ARG_RAM | ARG_REG | ARG_IMMED)) {
                if ((cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))] + INT_INSTRUCTIONS_BUFFER(ip + 2 * sizeof(commands)) >= sizeOfRam) || 
                    (cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))] + INT_INSTRUCTIONS_BUFFER(ip + 2 * sizeof(commands)) < 0)) {
                        
                    PRINT_ERROR(SEGMENTATION_FAULT);
                    return SEGMENTATION_FAULT;

                } else if (((int)cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))] + INT_INSTRUCTIONS_BUFFER(ip + 2 * sizeof(commands))) != 
                                (cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))] + INT_INSTRUCTIONS_BUFFER(ip + 2 * sizeof(commands)))) {
                    PRINT_ERROR(WRONG_ARGS);
                    return WRONG_ARGS;
                }

                push(&cpu->st, cpu->ram[(int)(cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))] + INT_INSTRUCTIONS_BUFFER(ip + 2 * sizeof(commands)))]);
                ip = 3 * sizeof(commands);
            }

        } else if (COMMAND_POP) {
            if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_POP | ARG_IMMED | ARG_RAM)) {
                elem_t element = 0.0;
                pop(&cpu->st, &element);

                if ((INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands)) >= sizeOfRam) || (INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands)) < 0)) {
                    PRINT_ERROR(SEGMENTATION_FAULT);
                    return SEGMENTATION_FAULT;
                }

                cpu->ram[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))] = element;
                ip += 2 * sizeof(commands);

            } else if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_POP | ARG_IMMED)) {
                elem_t element = 0.0;
                pop(&cpu->st, &element);
                ip += sizeof(commands);

            } else if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_POP | ARG_REG | ARG_RAM)) {
                elem_t element = 0.0;
                pop(&cpu->st, &element);

                if ((cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))] >= sizeOfRam) || (cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))] < 0)) {
                    PRINT_ERROR(SEGMENTATION_FAULT);
                    return SEGMENTATION_FAULT;

                } else if ((int)cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))] != cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))]) {
                    PRINT_ERROR(WRONG_ARGS);
                    return WRONG_ARGS;
                }

                cpu->ram[(int)cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))]] = element;
                ip += 2 * sizeof(commands);

            } else if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_POP | ARG_REG)) {
                elem_t element = 0.0;
                pop(&cpu->st, &element);
                cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))] = element;

                ip += 2 * sizeof(commands);

            } else if (INT_INSTRUCTIONS_BUFFER(ip) == (CMD_POP | ARG_RAM | ARG_REG | ARG_IMMED)) {
                if ((cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))] + INT_INSTRUCTIONS_BUFFER(ip + 2 * sizeof(commands)) >= sizeOfRam) || 
                    (cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))] + INT_INSTRUCTIONS_BUFFER(ip + 2 * sizeof(commands)) < 0)) {

                    PRINT_ERROR(SEGMENTATION_FAULT);
                    return SEGMENTATION_FAULT;

                } else if ((int)(cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))] + INT_INSTRUCTIONS_BUFFER(ip + 2 * sizeof(commands))) != 
                                (cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))] + INT_INSTRUCTIONS_BUFFER(ip + 2 * sizeof(commands)))) {
                    PRINT_ERROR(WRONG_ARGS);
                    return WRONG_ARGS;
                }

                elem_t element = 0.0;
                pop(&cpu->st, &element);

                cpu->ram[(int)(cpu->regs[INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands))] + INT_INSTRUCTIONS_BUFFER(ip + 2 * sizeof(commands)))] = element;
                ip += 3 * sizeof(commands);
            }
            
        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_ADD) {
            elem_t firstElement = 0.0;
            elem_t secondElement = 0.0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);
            push(&cpu->st, firstElement + secondElement);

            ip += sizeof(commands);

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_SUB) {
            elem_t firstElement = 0.0;
            elem_t secondElement = 0.0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);
            push(&cpu->st, firstElement - secondElement);

            ip += sizeof(commands);

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_MUL) {
            elem_t firstElement = 0;
            elem_t secondElement = 0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);
            push(&cpu->st, firstElement * secondElement);

            ip += sizeof(commands);

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_DIV) {
            elem_t firstElement = 0.0;
            elem_t secondElement = 0.0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);

            if (secondElement == 0.0) {
                PRINT_ERROR(DEVISION_BY_ZERO);
                return DEVISION_BY_ZERO;
            }

            push(&cpu->st, firstElement / secondElement);

            ip += sizeof(commands);

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_OUT) {
            elem_t element = 0.0;
            pop(&cpu->st, &element);
            printf("%lf\n", element);

            ip += sizeof(commands);

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_DUP) {
            elem_t element = 0.0;
            pop(&cpu->st, &element);
            push(&cpu->st, element);
            push(&cpu->st, element);

            ip += sizeof(commands);

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_JMP) {
            ip = INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands));

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_JB) {
            elem_t firstElement = 0.0;
            elem_t secondElement = 0.0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);

            if (firstElement < secondElement)
                ip = INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands));

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_JBE) {
            elem_t firstElement = 0.0;
            elem_t secondElement = 0.0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);

            if (firstElement <= secondElement)
                ip = INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands));

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_JA) {
            elem_t firstElement = 0.0;
            elem_t secondElement = 0.0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);

            if (firstElement > secondElement)
                ip = INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands));

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_JAE) {
            elem_t firstElement = 0.0;
            elem_t secondElement = 0.0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);

            if (firstElement >= secondElement)
                ip = INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands));

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_JE) {
            elem_t firstElement = 0.0;
            elem_t secondElement = 0.0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);

            if (firstElement == secondElement)
                ip = INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands));

        } else if (INT_INSTRUCTIONS_BUFFER(ip) == CMD_JNE) {
            elem_t firstElement = 0.0;
            elem_t secondElement = 0.0;

            pop(&cpu->st, &firstElement);
            pop(&cpu->st, &secondElement);

            if (firstElement != secondElement)
                ip = INT_INSTRUCTIONS_BUFFER(ip + sizeof(commands));

        } else {

        }
    }

    stack_destor(&cpu->st);
    free(cpu);
    return NOERRORS;
}

static struct my_cpu* cpu_constructor(const char* fileName, const char* functionName, unsigned int line) {
    struct my_cpu* cpu = (struct my_cpu*)calloc(1, sizeof(my_cpu));

    if (cpu == NULL) {
        PRINT_ERROR(RETURNED_NULL_COMMON);
        return NULL;
    }

    cpu->fileName = fileName;
    cpu->functionName = functionName;
    cpu->line = line;
    stack_ctor(&cpu->st, 2);
    
    for (long unsigned int regIndex = 0; regIndex < sizeof(cpu->regs) / sizeof(cpu->regs[0]); ++regIndex) 
        cpu->regs[regIndex] = 0;

    return cpu;
} 

static bool correct_signature(FILE* file) {
    int fileSignature[3] = { 0, 0, 0 };
    fread(fileSignature, sizeof(char), sizeof(fileSignature), file);

    if ((fileSignature[0] != SIGNATURE[0]) || (fileSignature[1] != SIGNATURE[1]) || (fileSignature[2] > SIGNATURE[2])) {
        PRINT_ERROR(WRONG_EXE_FILE);
        return false;
    }

    return true;
}

int main() {
    char* exeFilePath = get_name_stdin("Enter the executable file path: ");
    long unsigned int sizeOfBuffer = 0;

    char* instructionsBuffer = get_buffer(exeFilePath, &sizeOfBuffer);

    if (instructionsBuffer == NULL) 
        return -1;

    free(exeFilePath);

    allErrors error = run(instructionsBuffer, sizeOfBuffer);
    free(instructionsBuffer);

    if (error != NOERRORS) 
        return -1;

    return 0;
}


