/* Struct example
struct example {
    int id;
    const char* str;
};
*/

#define DOUBLE

#ifdef CANARY_PROT
    typedef long long unsigned int canary_t;
#endif

#ifdef INT
    typedef int elem_t;

#elif defined DOUBLE
    typedef double elem_t;

#else 
    typedef example elem_t;
#endif

/// enum type errors
enum errors {
    NO_IMPORTANT_ERRORS    = 1 << 0, ///< If no errors happened
    OVERFLOW               = 1 << 1, ///< If there was stack overflow (realloc returned NULL in push)
    UNDERFLOW              = 1 << 2, ///< If there was stack underflow (pop when size == 0) 
    RETURNED_NULL          = 1 << 3, ///< If realloc returned NULL in pop
    WRONG_PARAMETERS       = 1 << 4, ///< If wrong parameters were given to stack_constructor (stack_distructor)
    NULLPTR                = 1 << 5, ///< If in any function (except c-tor, dis-tor, dump) were given NULL as a parameter
    FILE_WASNT_OPEN        = 1 << 6, ///< If any file wasn`t open
    MISMACH_STRUCT_CANARY  = 1 << 7, ///< If canary in structure was broken
    MISMATCH_BUFFER_CANARY = 1 << 8, ///< If canary in buffer was broken
    MISMATCH_HASH          = 1 << 9, ///< If there was mismatch between current hash and st.hash field
    HAS_BEEN_DESTRUCTED    = 1 << 10, ///< If stack has already been destructed
    WASNT_CREATED          = 1 << 11, ///< If stack wasn`t created
    HAS_BEEN_CREATED       = 1 << 12, ///< If stack has already been created
    MISMATCH_SIZE          = 1 << 13, ///< If there was mismatch between sizes
    MISMATCH_CAPACITY      = 1 << 14, ///< If there was mismatch between capacities
};

struct size_and_capacity_info {
    unsigned int size;
    unsigned int capacity;
};

struct stack {
#ifdef CANARY_PROT
    canary_t leftCanary;
#endif
#ifdef HASH_PROT
    unsigned int hash;
#endif
    bool created = false;
    bool destroyed = false;
    struct size_and_capacity_info* info;
    elem_t* buffer;
    unsigned int size;
    unsigned int capacity; 
    const char* file;
    const char* function;
    int line;
    const char* name;
    long unsigned int error;
#ifdef CANARY_PROT
    canary_t rightCanary;
#endif
};

int stack_constructor(struct stack* st, const int cp, const char* name, const char* file, const char* function, const int line);

int stack_destructor(struct stack* st);

int stack_pop(struct stack* st, elem_t* element);

int stack_push(struct stack* st, const elem_t element);

#ifdef DEBUG
void debug(struct stack* st);
#endif

#define VARNAME(var) #var + (#var[0] == '&')

