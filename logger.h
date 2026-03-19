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

    /* --- PARSER (LR) --- */
    PARSE_SHIFT,
    PARSE_REDUCE_RULE,
    PARSE_REDUCE_RULE_COMPLETE,
    PARSE_ENTERING_STATE,
    PARSE_STACK_SNAPSHOT,
    PARSE_LOOKAHEAD_TOKEN,

    /* --- SEMANTIC / SYMBOL --- */
    PARSE_SEMANTIC_STEP,
    PARSE_ENTER_SCOPE,
    PARSE_CREATE_SCOPE,
    PARSE_EXIT_SCOPE,
    PARSE_ADD_SYM,
    PARSE_ASSGN_SYM_TYPE,
    PARSE_CREATE_AST_NODE,

    /* --- SEMANTIC LOGGING --- */
    SEMANTIC_PASS_STATUS,
    SEMANTIC_SYMBOL_HIGHLIGHT,
    SEMANTIC_NODE_HIGHLIGHT,
    SEMANTIC_ERROR_LOG,
    SEMANTIC_ANALYSIS_COMPLETE
} StepType;


typedef struct ReadToken {
    const char* tokenName;
    const char* value;
    int line_no;
    int char_no;
} ReadToken;

typedef struct ParseShift {
    const char* token;
    int from_state;
    int to_state;
} ParseShift;

typedef struct ParseStackSnapshot {
    int* states;
    int size;
} ParseStackSnapshot;

typedef struct ParseEnteringState {
    int state;
} ParseEnteringState;

typedef struct ReduceRule {
    int ruleNo;
    const char* rule;
} ReduceRule;

typedef struct ReduceRuleComplete {
    int ruleNo;
    const char* lhs;
    int rhsLength;
} ReduceRuleComplete;

typedef struct SemanticStep {
    int ruleNo;
    int stepNo;
    const char* instr;
} SemanticStep;

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

typedef struct CreateASTNode {
    int node_id;
} CreateASTNode;

typedef struct AssignSymType {
    const char* name;
    char* type;
    int scope_id;
} AssignSymType;

typedef struct SemanticPassStatus {
    const char* pass;
    const char* status;
    const char* message;
} SemanticPassStatus;

typedef struct SemanticSymbolHighlight {
    int scope_id;
    const char* symbol_name;
    const char* reason;
    const char* old_type;
    const char* new_type;
    int line_no;
    int char_no;
    int has_location;
} SemanticSymbolHighlight;

typedef struct SemanticNodeHighlight {
    const char* pass;
    int node_id;
    const char* node_type;
    int line_no;
    int char_no;
    const char* action;
    const char* message;
} SemanticNodeHighlight;

typedef struct SemanticErrorLog {
    const char* pass;
    const char* message;
    int line_no;
    int char_no;
    const char* node_id;
    const char* scope_id;
    const char* symbol_name;
} SemanticErrorLog;

typedef struct SemanticAnalysisComplete {
    const char* status;
    int total_errors;
} SemanticAnalysisComplete;

typedef struct Step {
    StepType type;
    union {
        ReadToken readToken;
        ReduceRule reduceRule;
        ReduceRuleComplete ReduceRuleComplete;
        SemanticStep SemanticStep;

        /* Parser (LR) */
        ParseShift ParseShift;
        ParseEnteringState ParseEnteringState;
        ParseStackSnapshot ParseStackSnapshot;

        /* Scope / symbols */
        EnterScope EnterScope;
        CreateScope CreateScope;
        ExitScope ExitScope;
        AddSymbol AddSymbol;
        AssignSymType AssignSymType;
        CreateASTNode CreateASTNode;
        SemanticPassStatus SemanticPassStatus;
        SemanticSymbolHighlight SemanticSymbolHighlight;
        SemanticNodeHighlight SemanticNodeHighlight;
        SemanticErrorLog SemanticErrorLog;
        SemanticAnalysisComplete SemanticAnalysisComplete;
    };
} Step;


// Functions
void init_logger();
void start_phase(PhaseType phase);
void log_step(Step);
void end_phase();
void close_logger();

#endif
