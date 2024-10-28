#ifndef SYMTABLE_H
#define SYMTABLE_H


// Forward declaration of ASTNode
typedef struct ASTNode ASTNode;
typedef struct SymbolTable SymbolTable;
// Symbol structure
typedef struct symbol {
    const char* name;      // Symbol name (e.g., variable or function name)
    char* type;      // Symbol type (e.g., int, float, etc.)
    SymbolTable* scope;       // Scope level (e.g., global = 0, local > 0)
    int location;    // Memory location or offset in the stack
    int is_function; // Whether the symbol is a function
} symbol;

// Symbol Table structure
typedef struct SymbolTable {
    int table_id;
    char* scopeName;         // Scope Name
    symbol** symbols;   // Dynamic array of pointers to symbols
    int size;           // Number of symbols in the table
    int capacity;       // Capacity of the table
    struct SymbolTable* parent; // Parent scope
    struct SymbolTable** children;
    int num_children;
} SymbolTable;

// Function declarations
SymbolTable* createSymbolTable(char* scopeName, SymbolTable* parent, int initial_capacity);
void addSymbol(SymbolTable* table, symbol* sym);
symbol* createSymbol(const char* name, char* type, SymbolTable* scope, int location, int is_function);
symbol* lookupSymbol(SymbolTable* table, const char* name);
void freeSymbolTable(SymbolTable* table);
void printSymbolTable(SymbolTable* table);
SymbolTable* enterScope(char* name, SymbolTable* currentScope);
SymbolTable* exitScope(SymbolTable* currentScope);

#endif
