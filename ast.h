#ifndef AST_H
#define AST_H

#include "symTable.h"
#include <stdbool.h>
typedef enum NodeType {
    NODE_PROGRAM,       // Represents the entire program
    NODE_STMT_LIST,     // List of statements
    NODE_STMT,
    NODE_DECL,     // Declaration statement
    NODE_ASSIGN_STMT,   // Assignment statement
    NODE_ASSGN,
    NODE_EXPR_STMT,     // Expression statement
    // NODE_COND_STMT,     // Conditional statement (if-else)
    NODE_IF,
    NODE_IF_ELSE,
    NODE_BLOCK_STMT,    // Block statement (enclosed in {})
    NODE_LOOP_STMT,     // Loop statements (while, for)
    NODE_FOR,
    NODE_WHILE,
    NODE_FUNC_DECL,     // Function declaration
    NODE_FUNC_CALL,     // Function call
    NODE_VAR_LIST,      // Variable declaration
    NODE_VAR,           // Variable (identifier)
    NODE_TYPE_SPEC,     // Type specification (int, char, etc.)
    NODE_EXPR_BINARY,          // General expression Node
    NODE_EXPR_UNARY,
    NODE_EXPR_TERM,    
    NODE_OP,            // All operations (binary, logical, comparison, etc.)
    NODE_UNARY_OP,      // Unary operations (e.g., -, ++, --)
    NODE_INT_LITERAL,   // Integer literal
    NODE_CHAR_LITERAL,  // Character literal
    NODE_STR_LITERAL,   // String literal
    NODE_ID,            // Identifier (variable names)
    NODE_ID_REF,        // Identifier Reference node 
    NODE_ARG_LIST,      // Argument list for function calls
    NODE_ARRAY_DECL,
    NODE_PARAM_LIST,    // Parameter list for function declarations
    NODE_PARAM,
    NODE_EXPR_LIST,      // List of expressions in for loop
    NODE_RETURN,
    NODE_COMMA,
    NODE_EMPTY
} NodeType;




typedef struct ASTNode {
    NodeType type;  // Type of the node (enum to identify node type)

    union {

        struct {
          struct ASTNode* stmt_list;
        } program_data;




        // Function decl node data
        struct {
            symbol* sym;                    // Function symbol (name)
            struct ASTNode** params;        // List of params (can be NULL if none)
            struct ASTNode* body;           // Function body (block of statements)
            struct ASTNode* return_type;    // Return type (optional, can be NULL)
        } func_decl_data;

        // Function call node data
        struct {
            symbol* sym;                   // Function symbol (name)
            struct ASTNode** args;    // Array of arguments passed to the function
            int arg_count;                 // Number of arguments
        } func_call_data;

        // Assignment node data
        struct {
            symbol* sym;                  // The variable being assigned to
            struct ASTNode* value;        // The value being assigned
        } assignment_data;

        // Literal node data (for integers, characters, strings, etc.)
        struct {
            union{
                int int_value;
                char char_value;            
                const char* str_value;            // Literal value as a string (or could store numeric types if needed)
            } value;
        } literal_data;

        
        

        // Binary operator node data (for operations like +, -, *, /, etc.)
        struct {
            const char* op;               // Operator, e.g. "+", "-", "=="
            struct ASTNode* left;         // Left operand
            struct ASTNode* right;        // Right operand
        } expr_data;

        // If-Else node data (for conditional statements)
        struct {
            struct ASTNode* condition;    // The condition expression
            struct ASTNode* if_branch;    // Code to execute if condition is true
            struct ASTNode* else_branch;  // Code to execute if condition is false (can be NULL)
        } if_else_data;

        // For loop node data
        struct {
            struct ASTNode* init;         // Initialization statement
            struct ASTNode* condition;    // Loop condition
            struct ASTNode* increment;    // Increment statement
            struct ASTNode* body;         // The body of the for loop
        } for_data;
        
        // While loop node data
        struct {
            struct ASTNode* condition;    // The condition expression
            struct ASTNode* while_branch; // The body of the while loop
        } while_data;

        // Declarations

        struct {
            struct ASTNode* type_spec;
            struct ASTNode* var_list;
        } decl_data;

        struct{
          struct ASTNode* var_list;
          struct ASTNode* var;
        } var_list_data;

        struct{
          struct ASTNode* id;
          struct ASTNode* value;
        } var_data;

        
        struct{
            const char* type;
        } type_data;

        struct {
            symbol* sym;                  // The referenced variable symbol
        } id_data;

        struct {
            const char *name;
            symbol* ref;
        } id_ref_data;
        
        struct {
            struct ASTNode* left;
            struct ASTNode* right;
        } assgn_data;
        // Statements
        struct {
            struct ASTNode* stmt_list;
            struct ASTNode* stmt;
        } stmt_list_data;
        
        struct {
            struct ASTNode* stmt;
        } stmt_data;

        // Block statement (contains multiple statements)
        struct {
            struct ASTNode* stmt_list;  // Array of statements (children)
        } block_stmt_data;

        
        // Return node data (for return statements in functions)
        struct {
            struct ASTNode* return_value; // The expression being returned (can be NULL for "void" return)
        } return_data;
    };

    // // For nodes that need multiple children (e.g., block statements, function bodies)
    // struct ASTNode** children;
    // int child_count;

} ASTNode;


// Function prototypes for AST operations
ASTNode* createASTNode(NodeType);
void printAST(ASTNode* node, int indent, bool isLast);
void freeAST(ASTNode* node);


#endif // AST_H
