#ifndef ICG_H
#define ICG_H

#include "ast.h"
#include "symTable.h"
#include "semantic.h"

typedef enum {
    TAC_ADD,
    TAC_SUB,
    TAC_MUL,
    TAC_DIV,
    TAC_AND,
    TAC_OR,
    TAC_NOT,
    TAC_NEG,
    TAC_EQ,
    TAC_NEQ,
    TAC_LT,
    TAC_GT,
    TAC_LEQ,
    TAC_GEQ,
    TAC_POST_INC,
    TAC_POST_DEC,
    TAC_PRE_DEC,
    TAC_PRE_INC,
    TAC_ASSIGN,
    TAC_LABEL,
    TAC_GOTO,
    TAC_IF_GOTO,
    TAC_IF_FALSE_GOTO,
    TAC_CALL,
    TAC_PARAM,
    TAC_RETURN,
} TACOp;

typedef enum{
    INT_VAL,
    CHAR_VAL,
    STR_VAL,
    ID_REF
} ValueType;

typedef enum{
    INT_TO_CHAR,
    CHAR_TO_INT,
    STR_TO_INT,
} TypeConversion;

typedef struct Label Label;
typedef struct List List;


typedef struct Operand{
    ValueType type;
    TypeConversion typeConv;

    union{
        int int_val;
        char char_val;
        char* str_val;
        struct{
            char* name;
            symbol* sym;
        }id_ref;
    };

} Operand;

typedef struct TAC {
    int tac_id;
    Label* label; // Label of this TAC

    TACOp op;
    const char* result;
    Operand* operand1;
    Operand* operand2;

    List* truelist;
    List* falselist;

    Label* target_label;
    int target_jump;

    struct TAC* next;  // Linked list of instructions
} TAC;

typedef struct TACList {
    TAC* head;
    TAC* tail;
} TACList;

typedef struct List {
    struct TAC* tac;    // Pointer to the TAC instruction to be patched later
    struct List* next;   // Pointer to the next instruction in the list
} List;

typedef struct Label {
    char* name;      // The name of the label, e.g., "L1"
    struct TAC* tac; // Pointer to the TAC instruction where the label is used
} Label;


typedef struct BoolExprInfo {
    List* trueList;
    List* falseList;
} BoolExprInfo;

void setICGDebugger();
void startICG(ASTNode* root);

TACList* createTACList(); 
// Function to create a new temporary variable
char* newTempVar();

// Function to create a new label
char* newLabel();

// Functions to generate TAC code for expressions, assignments, etc.
const char* getOperatorString(TACOp op);
TAC* generateCode(ASTNode* node, BoolExprInfo* bool_info);
void attachValueOfExprTerm(ASTNode* node, Operand** opr);
TAC* generateCodeForBinaryExpr(ASTNode* node, BoolExprInfo* bool_info);
TAC* generateCodeForAssignment(ASTNode* node);

// Function to create a TAC instruction
TAC* createTAC(TACOp op, char* result, Operand* operand1, Operand* operand2);

// Function to print the generated TAC
void printTAC();


#endif