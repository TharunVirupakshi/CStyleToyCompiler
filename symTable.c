#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symTable.h"

// Create a new symbol table
SymbolTable* createSymbolTable(int initial_capacity) {
    
    SymbolTable* table = (SymbolTable*)malloc(sizeof(SymbolTable));

    if (!table) {
        printf("Memory allocation for symbol table failed\n");
        exit(1);
    }

    table->symbols = (symbol**)malloc(sizeof(symbol*) * initial_capacity);
    table->size = 0;
    table->capacity = initial_capacity;
    return table;
}

// Create a new symbol
symbol* createSymbol(char* name, char* type, int scope, int location, int is_function) {
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
    printf("Added symbol: %s\n", sym->name);  // Add this
}

// Look up a symbol in the symbol table by name
symbol* lookupSymbol(SymbolTable* table, char* name) {
    for (int i = 0; i < table->size; i++) {
        if (strcmp(table->symbols[i]->name, name) == 0) {
            return table->symbols[i];  // Return the symbol if found
        }
    }
    return NULL;  // Symbol not found
}

// Free the memory allocated for the symbol table
void freeSymbolTable(SymbolTable* table) {
    for (int i = 0; i < table->size; i++) {
        free(table->symbols[i]->name);
        free(table->symbols[i]->type);
        free(table->symbols[i]);
    }
    free(table->symbols);
    free(table);
}

// Print the symbol table
void printSymbolTable(SymbolTable* table) {
    printf("Symbol Table:\n");
    printf("--------------------------------------------------------\n");
    printf("| %-10s | %-10s | %-5s | %-8s | %-10s |\n", "Name", "Type", "Scope", "Location", "Function?");
    printf("--------------------------------------------------------\n");

    for (int i = 0; i < table->size; i++) {
        symbol* sym = table->symbols[i];
        printf("| %-10s | %-10s | %-5d | %-8d | %-10s |\n",
               sym->name, 
               sym->type, 
               sym->scope, 
               sym->location, 
               sym->is_function ? "Yes" : "No");
    }

    printf("--------------------------------------------------------\n");
}