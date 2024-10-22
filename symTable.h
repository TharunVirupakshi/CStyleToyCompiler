#ifndef SYMTABLE_H
#define SYMTABLE_H

// Symbol structure
typedef struct symbol {
    char* name;      // Symbol name (e.g., variable or function name)
    char* type;      // Symbol type (e.g., int, float, etc.)
    int scope;       // Scope level (e.g., global = 0, local > 0)
    int location;    // Memory location or offset in the stack
    int is_function; // Whether the symbol is a function
} symbol;

// Symbol Table structure
typedef struct SymbolTable {
    symbol** symbols;   // Dynamic array of pointers to symbols
    int size;           // Number of symbols in the table
    int capacity;       // Capacity of the table
} SymbolTable;

// Function declarations
SymbolTable* createSymbolTable(int initial_capacity);
void addSymbol(SymbolTable* table, symbol* sym);
symbol* createSymbol(char* name, char* type, int scope, int location, int is_function);
symbol* lookupSymbol(SymbolTable* table, char* name);
void freeSymbolTable(SymbolTable* table);
void printSymbolTable(SymbolTable* table);

#endif
