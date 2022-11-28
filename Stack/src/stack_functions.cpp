#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "stack_functions.h"

#define dump stack_dump(st, __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define my_assert_if(condition, error, arg) if (condition) {processor_of_errors(error, __FILE__, __PRETTY_FUNCTION__, __LINE__); return arg} 

static void print_elements(int* buffer, FILE* stream, unsigned int capacity);
static void print_elements(double* buffer, FILE* stream, unsigned int capacity);
static void print_elements(elem_t* buffer, FILE* stream, unsigned int capacity);

static const long unsigned int poison = 0XDEADDEAD;

static const char logFile[] = "Output_files/log.txt"; 

#ifdef DEBUG
const char debugFile[] = "output_files/debug.txt";
#endif

#ifdef CANARY_PROT
    static const canary_t canaryDefinition = 0XBAADF00D;
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

static void processor_of_errors(errors error, const char* fileName, const char* functionName, const int line) { 
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
    if (st->error & MISMATCH_CAPACITY) 
        fprintf(stream, "   Mismatch between real capacity and check capacity\n");
    if (st->error & MISMATCH_SIZE)
        fprintf(stream, "   Mismatch between size capacity and check size\n");
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
        fprintf(stream, "Left canary in buffer = %llX\n", *((canary_t*)st->buffer)); 
        fprintf(stream, "Right canary in buffer = %llX\n", *((canary_t*)((char*)st->buffer + sizeof(canaryDefinition) + sizeof(elem_t) * st->capacity)));

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

static int verification(struct stack* st) {
    my_assert_if(st == NULL, NULLPTR, NULLPTR;) 
    bool errorFound = false;

    if (st->capacity != st->info->capacity) {
        st->error |= MISMATCH_CAPACITY;
        errorFound = true;
    }

    if (st->size != st->info->size) {
        st->error |= MISMATCH_SIZE;
        errorFound = true;
    }

#ifdef CANARY_PROT
    canary_t leftCanary = st->leftCanary;
    canary_t rightCanary = st->rightCanary;

    if ((leftCanary != canaryDefinition) || (rightCanary != canaryDefinition)) {
        st->error |= MISMACH_STRUCT_CANARY;
        errorFound = true;
    }
    
    if (((*((canary_t*)st->buffer)) != canaryDefinition) || 
        ((*((canary_t*)((char*)st->buffer + sizeof(canaryDefinition) + st->capacity * sizeof(elem_t)))) != canaryDefinition)) {
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

    if (errorFound) 
        dump;

    return st->error;
}

static errors resize(struct stack* st, const unsigned int oldCapacity, const unsigned int newCapacity) {
    my_assert_if(st == NULL, NULLPTR, NULLPTR;) 

#ifdef CANARY_PROT
    st->buffer = (elem_t*)realloc(st->buffer, newCapacity * sizeof(elem_t) + 2 * sizeof(canaryDefinition));    
    
    if (st->buffer == NULL) 
        return RETURNED_NULL;

    *((canary_t*)st->buffer) = canaryDefinition; 
    *((canary_t*)((char*)st->buffer + sizeof(canaryDefinition) + newCapacity * sizeof(elem_t))) = canaryDefinition;

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

int stack_constructor(struct stack* st, const int cp, const char* name, const char* file, const char* function, const int line) {
    my_assert_if(st == NULL, NULLPTR, NULLPTR;) 

    if (st->created) {
        st->error |= HAS_BEEN_CREATED;
        dump;
        return NO_IMPORTANT_ERRORS;
    }

    if ((cp <= 0) || (file == NULL) || (function == NULL) || (name == NULL)) {
        st->error = NO_IMPORTANT_ERRORS;
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
    st->error = NO_IMPORTANT_ERRORS;
    st->created = true;

    st->info = (struct size_and_capacity_info*)calloc(1, sizeof(struct size_and_capacity_info));
    if (st->info == NULL) {
        st->error |= RETURNED_NULL;
        dump;
        return RETURNED_NULL;
    }

    st->info->capacity = capacity;
    st->info->size = 0;

#ifdef CANARY_PROT
    st->rightCanary = canaryDefinition;
    st->leftCanary = canaryDefinition;

    st->buffer = (elem_t*)calloc(capacity * sizeof(elem_t) + 2 * (sizeof(canaryDefinition)), sizeof(char));

    if (st->buffer == NULL) {
        st->error |= RETURNED_NULL;
        dump;
        return RETURNED_NULL;
    }   

    *((canary_t*)st->buffer) = canaryDefinition; 
    *((canary_t*)((char*)st->buffer + sizeof(canaryDefinition) + capacity * sizeof(elem_t))) = canaryDefinition;
    
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

    return verification(st);
}

static void stack_not_created(struct stack* st) {
    my_assert_if(st == NULL, NULLPTR, ;)

    printf("Look at the output_files/log.txt\n");

    FILE* stream = fopen(logFile, "w");
    my_assert_if(!file_is_open(stream), FILE_WASNT_OPEN, ;)
    fprintf(stream, "Stack at adress %p wasn`t created\n", st);
    fclose(stream);
}

int stack_push(struct stack* st, const elem_t element) {
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

    int error = verification(st);
    if (error != NO_IMPORTANT_ERRORS)
        return error;

    unsigned int size = st->size;
    unsigned int capacity = st->capacity;
    
    if (size >= capacity) {
        errors errorInResize = resize(st, capacity, capacity * 2);

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

        st->info->capacity = capacity;
        st->info->size = size;

        return verification(st);   
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

    st->info->capacity = capacity;
    st->info->size = size;

    return verification(st);
}

int stack_pop(struct stack* st, elem_t* element) {
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

    int error = verification(st);
    if (error != NO_IMPORTANT_ERRORS)
        return error;

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
        errors errorInResize = resize(st, capacity, capacity / 2); 

        if (errorInResize == NULLPTR) {
            return NULLPTR;

        } else if (errorInResize == RETURNED_NULL) {
            st->error |= RETURNED_NULL;
            dump;
            return RETURNED_NULL;
        }

        capacity /= 2;

        st->capacity = capacity;
    }

#ifdef HASH_PROT
    st->hash = HashRot13(st->buffer, st->capacity * sizeof(elem_t));
#endif

    st->info->capacity = capacity;
    st->info->size = size;

    return verification(st);
}

int stack_destructor(struct stack* st) {
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

    st->size = 0;
    st->capacity = 0;
    st->info->capacity = 0;
    st->info->size = 0;
    free(st->info);
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

    st->error = NO_IMPORTANT_ERRORS;
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

    fprintf(stream, "Left canary in buffer = %llX\n", *((canary_t*)st->buffer)); 
    fprintf(stream, "Right canary in buffer = %llX\n", *((canary_t*)((char*)st->buffer + sizeof(canaryDefinition) + sizeof(elem_t) * st->capacity)));
    

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



