#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symTable.h"

// Function to perform semantic analysis
void performSemanticAnalysis(ASTNode* root, SymbolTable* globalTable);

// Helper function to validate symbol declarations for id_ref nodes
// void validateSymbolUsage(ASTNode* id_ref, SymbolTable* currentScope);

void checkDuplicates(SymbolTable* table);

#endif