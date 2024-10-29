#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printErrors();

// Error structure
typedef struct Error {
    char* message;
    int line;
    int char_no;
} Error;

extern bool isSemanticError;

// Error list
Error* errorList = NULL;
int errorCount = 0;

// Main function
SemanticStatus performSemanticAnalysis(ASTNode* root, SymbolTable* globalTable) {
    if (!root) return SEMANTIC_ERROR;
    checkDuplicates(globalTable);
    // validateSymbolUsage(root, globalTable);

    // print errors if any
    if(errorList){
        printErrors();
    } 

    return errorCount > 0 ? SEMANTIC_ERROR : SEMANTIC_SUCCESS;
}





// Add an error to the list
void addError(const char* message, int line, int char_no) {
    errorList = realloc(errorList, (errorCount + 1) * sizeof(Error));
    if (!errorList) {
        fprintf(stderr, "Memory allocation failed for error list\n");
        exit(1);
    }
    errorList[errorCount].message = strdup(message);
    errorList[errorCount].line = line;
    errorList[errorCount].char_no = char_no;
    errorCount++;
}

// Function to print all stored errors
void printErrors() {
    for (int i = 0; i < errorCount; i++) {
        printf("Error: %s at line %d, char %d\n", errorList[i].message, errorList[i].line, errorList[i].char_no);
        free(errorList[i].message); // Free each error message after printing
    }
    free(errorList); // Free the entire list at the end
    errorList = NULL; // Reset the list pointer
 
}

// Modified checkDuplicates function
void checkDuplicates(SymbolTable* table) {
    if (!table) return;

    for (int i = 0; i < table->size; ++i) {
        if(table->symbols[i]->is_duplicate) continue;

        for (int j = i+1; j < table->size; ++j) {
            
            if (strcmp(table->symbols[i]->name, table->symbols[j]->name) == 0) {
                table->symbols[j]->is_duplicate = 1;
                char errorMsg[256];
                snprintf(errorMsg, sizeof(errorMsg), "Duplicate declaration of '%s' (previously declared at line %d, char %d)",
                    table->symbols[j]->name,
                    table->symbols[i]->line_no,
                    table->symbols[i]->char_no);
                addError(errorMsg, table->symbols[j]->line_no, table->symbols[j]->char_no);
            }
        }
    }

    // Check child scopes recursively
    for (int k = 0; k < table->num_children; ++k) {
        checkDuplicates(table->children[k]);
    }
}


