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


// Function to perform semantic analysis
SemanticStatus performSemanticAnalysis(ASTNode* root, SymbolTable* globalTable);

// Helper function to validate symbol declarations for id_ref nodes
void validateSymbolUsage(ASTNode* root);
void checkDuplicates(SymbolTable* table);
void validateTypes(ASTNode* root);
void validateFunctionReturnTypes(ASTNode* root);
void setSemanticDebugger();
OpType getOpType(const char* op);

#endif