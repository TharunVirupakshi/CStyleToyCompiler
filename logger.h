#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

// Enum for phases
typedef enum {
    PHASE_LEX_PARSE,
    PHASE_SEMANTIC,
    PHASE_ICG
} PhaseType;

typedef enum {
    LEX_READ_TOKEN,
    PARSE_REDUCE_RULE,
    PARSE_ENTER_SCOPE,
    PARSE_CREATE_SCOPE,
    PARSE_EXIT_SCOPE,
    PARSE_ADD_SYM
} StepType;

typedef struct ReadToken {
    const char* tokenName;
    const char* value;
    int line_no;
    int char_no;
} ReadToken;

typedef struct ReduceRule {
    const char* rule;
} ReduceRule;

typedef struct CreateScope {
    int table_id;
    const char* scopeName;
    int parent_id; 
} CreateScope;

typedef struct EnterScope {
    int table_id;
    const char* scopeName;
} EnterScope;

typedef struct ExitScope {
    int table_id;
    const char* scopeName;    
} ExitScope;

typedef struct AddSymbol {
    const char* name;
    char* type;
    int scope_id;
    int is_function;
    int line_no;
    int char_no;
    int is_duplicate;
} AddSymbol;

typedef struct Step {
    StepType type;
    union {
        ReadToken readToken;
        ReduceRule reduceRule;
        EnterScope EnterScope;
        CreateScope CreateScope;
        ExitScope ExitScope;
        AddSymbol AddSymbol;
    };
} Step;

// Functions
void init_logger();
void start_phase(PhaseType phase);
void log_step(Step);
void end_phase();
void close_logger();

#endif
