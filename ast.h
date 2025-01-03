#ifndef AST_H
#define AST_H

#include "symTable.h"
#include <stdbool.h>
#include <stdio.h>

#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir _mkdir
#else
#include <sys/stat.h>
#endif

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
    NODE_IF_COND,
    NODE_IF_BRANCH,
    NODE_ELSE_BRANCH,
    NODE_BLOCK_STMT,    // Block statement (enclosed in {})
    NODE_LOOP_STMT,     // Loop statements (while, for)
    NODE_FOR,
    NODE_FOR_INIT,
    NODE_FOR_COND,
    NODE_FOR_UPDATION,
    NODE_FOR_BODY,
    NODE_EXPR_COMMA_LIST,
    NODE_WHILE,
    NODE_WHILE_COND,
    NODE_WHILE_BODY,
    NODE_FUNC_DECL,     // Function declaration
    NODE_FUNC_BODY,
    NODE_FUNC_CALL,     // Function call
    NODE_ARG_LIST,      // Argument list for function calls
    NODE_ARG,
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
    NODE_ARRAY_DECL,
    NODE_PARAM_LIST,    // Parameter list for function declarations
    NODE_PARAM,
    NODE_EXPR_LIST,      // List of expressions in for loop
    NODE_RETURN,
    NODE_BREAK_STMT,
    NODE_CONTINUE_STMT,
    NODE_COMMA,
    NODE_EMPTY
} NodeType;

const char* getNodeName(NodeType type); 
void setASTDebugger();

typedef struct ASTNode {
    NodeType type;  // Type of the node (enum to identify node type)
    int node_id;
    int line_no, char_no;
    SymbolTable* scope;

    const char* inferedType;
    union {

        struct {
          struct ASTNode* stmt_list;
          SymbolTable* scope;
        } program_data;




        // Function decl node data
        struct {
            struct ASTNode* id;           // Function identifier
            struct ASTNode* params;       // Root of the parameter list binary tree
            struct ASTNode* body;         // Function body (block of statements)
            int param_count;              // Number of parameters
            SymbolTable* scope;
        } func_decl_data;
        
        struct {
            struct ASTNode* body;
        } func_body_data;

        struct {
            struct ASTNode* param_list;
            struct ASTNode* param;
        } param_list_data;

        struct {
            struct ASTNode* type_spec;
            struct ASTNode* id;
        } param_data;


        // Function call node data
        struct {
            ASTNode* id;
            ASTNode* arg_list;
            int arg_count;                 // Number of arguments
        } func_call_data;

        struct {
            ASTNode* arg_list;
            ASTNode* arg;
        } arg_list_data;

        struct {
            ASTNode* arg;
        } arg_data;

        // // Assignment node data
        // struct {
        //     symbol* sym;                  // The variable being assigned to
        //     struct ASTNode* value;        // The value being assigned
        // } assignment_data;

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

        struct {
            struct ASTNode* cond;
        } if_cond_data;

        struct {
            struct ASTNode* branch;
        } if_else_branch;

        // For loop node data
        struct {
            struct ASTNode* init;         // Initialization statement
            struct ASTNode* condition;    // Loop condition
            struct ASTNode* updation;    // Increment statement
            struct ASTNode* body;         // The body of the for loop
        } for_data;

        struct{
            struct ASTNode* init;
        } for_init_data;

        struct{
            struct ASTNode* cond;
        } for_cond_data;

        struct{
            struct ASTNode* updation;
        } for_updation_data;

        struct {
            struct ASTNode* body;
        } for_body_data;

        // Expr Comma List

        struct {
            struct ASTNode* expr_comma_list;
            struct ASTNode* expr_comma_list_item;
        } expr_comma_list_data;
        
        // While loop node data
        struct {
            struct ASTNode* condition;    // The condition expression
            struct ASTNode* while_body; // The body of the while loop
        } while_data;

        struct {
            struct ASTNode* cond;
        } while_cond_data;

        struct {
            struct ASTNode* body;
        } while_body_data;

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
            SymbolTable* scope; // Usage's scope
            int line_no;
            int char_no;
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
            struct ASTNode* associated_node; 
        } return_data;

        struct {
            struct ASTNode* associated_loop_node;
        } break_continue_stmt_data;

    };

} ASTNode;


// Function prototypes for AST operations
ASTNode* createASTNode(NodeType, int line_no, int char_no);
void printAST(ASTNode* node, int indent, bool isLast);
void freeAST(ASTNode* node);
void exportASTAsJSON(const char *folderPath, ASTNode *root);

typedef int (*ASTTraversalCallback)(ASTNode* node, void* context);
void traverseAST(ASTNode* node, ASTTraversalCallback callback, void* context);
const char* getDataTypeFromAST(ASTNode* node);

#endif // AST_H
