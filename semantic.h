#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symTable.h"

typedef enum {
    SEMANTIC_SUCCESS,
    SEMANTIC_ERROR
} SemanticStatus;


// Function to perform semantic analysis
SemanticStatus performSemanticAnalysis(ASTNode* root, SymbolTable* globalTable);

// Helper function to validate symbol declarations for id_ref nodes
void validateSymbolUsage(ASTNode* root);

void checkDuplicates(SymbolTable* table);

#endif