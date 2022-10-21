#define INT

#ifdef INT
    typedef int elem_t;

#elif defined DOUBLE
    typedef double elem_t;

#else 
    typedef example elem_t;
#endif

/// enum type errors
enum stack_errors {
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

