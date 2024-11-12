#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symTable.h"

typedef enum {
    SEMANTIC_SUCCESS,
    SEMANTIC_ERROR
} SemanticStatus;

typedef enum{
    OP_ARITHMETIC,
    OP_COMP,
    OP_LOGICAL,
    OP_INC_DEC,
    OP_UNKNOWN
}OpType;

typedef struct BrkCntStmtsList{
    ASTNode* node;
    struct BrkCntStmtsList* next;
}BrkCntStmtsList;


// Function to perform semantic analysis
SemanticStatus performSemanticAnalysis(ASTNode* root, SymbolTable* globalTable, BrkCntStmtsList* list);

// Helper function to validate symbol declarations for id_ref nodes
void validateSymbolUsage(ASTNode* root);
void checkDuplicates(SymbolTable* table);
void validateLoops(ASTNode* node, BrkCntStmtsList* list);
void validateTypes(ASTNode* root);
void validateFunctionReturnTypes(ASTNode* root);
void setSemanticDebugger();
OpType getOpType(const char* op);

#endif