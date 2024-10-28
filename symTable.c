#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symTable.h"
#define INITIAL_LOCAL_SCOPE_CAPACITY 20

int cur_table_id = 0;

// Create a new symbol table
SymbolTable* createSymbolTable(char* scopeName, SymbolTable* parent, int initial_capacity) {
    
    SymbolTable* table = (SymbolTable*)malloc(sizeof(SymbolTable));

    if (!table) {
        printf("Memory allocation for symbol table failed\n");
        exit(1);
    }
    table->table_id = cur_table_id++;
    table->scopeName = scopeName;
    table->symbols = (symbol**)malloc(sizeof(symbol*) * initial_capacity);
    table->size = 0;
    table->capacity = initial_capacity;
    table->parent = parent;
    // printf("Created SymTable: %s\n", name);
    return table;
}

// Create a new symbol
symbol* createSymbol(const char* name, char* type, SymbolTable* scope, int location, int is_function) {
    // printf("Creating symbol: %s\n", name);  
    symbol* sym = (symbol*)malloc(sizeof(symbol));
    if (!sym) {
        printf("Memory allocation for symbol failed\n");
        exit(1);
    }

    sym->name = strdup(name);    // Copy the name of the variable
    sym->type = type ? strdup(type) : NULL;  
    sym->scope = scope;          // Assign the current scope
    sym->location = location;          // Placeholder for memory location
    sym->is_function = is_function;        // Set as variable (not a function initially)
    // printf("Created symbol: %s\n", name);   
    return sym;
}

// Add a symbol to the symbol table
void addSymbol(SymbolTable* table, symbol* sym) {
    
    if (table->size == table->capacity) {
        table->capacity *= 2;
        table->symbols = (symbol**)realloc(table->symbols, sizeof(symbol*) * table->capacity);
        if (!table->symbols) {
            printf("Memory reallocation failed\n");
            exit(1);
        }
    }
    table->symbols[table->size++] = sym;
    // printf("Added symbol: %s\n", sym->name);  // Add this
}

// Look up a symbol in the current scope, searching up to parent scopes if not found
symbol* lookupSymbol(SymbolTable* table, const char* name) {
    SymbolTable* current = table;
    while (current) {
        for (int i = 0; i < current->size; i++) {
            if (strcmp(current->symbols[i]->name, name) == 0) {
                return current->symbols[i];
            }
        }
        current = current->parent;  // Move to the parent scope
    }
    return NULL;  // Symbol not found in any accessible scope
}


// Scope management
SymbolTable* enterScope(char* name, SymbolTable* currentScope) {
    SymbolTable* newScope = createSymbolTable(name, currentScope, INITIAL_LOCAL_SCOPE_CAPACITY);

    if(currentScope){
        currentScope->children = realloc(currentScope->children, sizeof(SymbolTable*) * (currentScope->num_children) + 1);
        currentScope->children[currentScope->num_children++] = newScope;
    }

    return newScope;
}

SymbolTable* exitScope(SymbolTable* currentScope) {
    if (currentScope == NULL || currentScope->parent == NULL) {
        // Stay in the global scope if there's no parent to go back to
        return currentScope;
    }
    return currentScope->parent;  // Move up one level in the scope chain
}

// Free symbol table (recursive free up the scope chain if needed)
void freeSymbolTable(SymbolTable* table) {
    if (!table) return;
    for (int i = 0; i < table->size; i++) {
        free((char*)table->symbols[i]->name);
        free(table->symbols[i]->type);
        free(table->symbols[i]);
    }
    free(table->symbols);
    freeSymbolTable(table->parent); // Free parent scope if needed
    free(table);
}

// Print the symbol table and all child tables recursively
void printSymbolTable(SymbolTable* table) {
    if (!table) return;

    // Header for the current symbol table
    printf("Symbol Table: %s [%d] | ParentScope(%s [%d]) \n", table->scopeName, table->table_id, table->parent ? table->parent->scopeName : "NULL", table->parent ? table->parent->table_id : -1);
    printf("-----------------------------------------------------------------------------------\n");
    printf("| %-20s | %-12s | %-18s | %-10s | %-10s |\n", "Name", "Type", "Scope", "Location", "Function?");
    printf("-----------------------------------------------------------------------------------\n");

    // Print each symbol in the table
    for (int i = 0; i < table->size; i++) {
        symbol* sym = table->symbols[i];
        static char scopeInfo[64];
        snprintf(scopeInfo, sizeof(scopeInfo), "%s [%d]", sym->scope->scopeName, sym->scope->table_id);
        printf("| %-20s | %-12s | %-18s | %-10d | %-10s |\n",
               sym->name, 
               sym->type, 
               scopeInfo,
               sym->location, 
               sym->is_function ? "Yes" : "No");
    }
    
    // End of table
    printf("-----------------------------------------------------------------------------------\n\n");

    // Recursively print each child symbol table
    for (int i = 0; i < table->num_children; ++i) {
        printSymbolTable(table->children[i]);
    }
}
