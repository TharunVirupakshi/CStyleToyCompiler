#include "symTable.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



// Function to create an empty AST node
ASTNode* createASTNode(NodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }

    node->type = type;
    // node->child_count = childCount;

    // Allocate memory for children only if childCount > 0
    // if (childCount > 0) {
    //     node->children = (ASTNode**)malloc(sizeof(ASTNode*) * childCount);
    //     if (!node->children) {
    //         fprintf(stderr, "Memory allocation error\n");
    //         free(node);
    //         exit(1);
    //     }
    //     // Initialize children to NULL
    //     for (int i = 0; i < childCount; i++) {
    //         node->children[i] = NULL;
    //     }
    // } else {
    //     node->children = NULL; // No children if count is zero
    // }


    return node;
}



// Function to print the AST recursively
void printAST(ASTNode* node, int indent) {
    if (node == NULL) return;

   // Print indentation, vertical lines, and dashes
    for (int i = 0; i < indent; i++) {
        if (i == indent - 1) {
            printf("|-- ");
        } else {
            printf("|   ");
        }
    }


    switch (node->type) {
        case NODE_PROGRAM:
            printf("PROGRAM\n");
            printAST(node->program_data.stmt_list, indent + 1);
            break;

        case NODE_RETURN:
            printf("RETURN\n");
            printAST(node->return_data.return_value, indent + 2); // Recursively print the return value
            break;

        case NODE_STR_LITERAL:
            printf("LITERAL (type: String, value: %s)\n", node->literal_data.value.str_value);
            break;

        case NODE_STMT_LIST:
            printf("STMT LIST\n");
            if (node->stmt_list_data.stmt_list) {
                printAST(node->stmt_list_data.stmt_list, indent + 1); // Print the statement list
            }
            if (node->stmt_list_data.stmt) {
                printAST(node->stmt_list_data.stmt, indent + 1);      // Print the individual statement
            }
            break;

        case NODE_STMT:
            printf("STMT\n");
            if (node->stmt_data.stmt) {
                printAST(node->stmt_data.stmt, indent + 1);          // Recursively print the statement
            }
            break;
        
        case NODE_DECL:
            printf("DECL\n");
            ASTNode* type_spec = node->decl_data.type_spec; 
            ASTNode* var_list = node->decl_data.var_list; 
            printAST(type_spec, indent+1);
            printAST(var_list, indent+1);
            break;

        case NODE_TYPE_SPEC:
            printf("TYPE_SEPC (type: %s)\n", node->type_data.type);
            break;

        case NODE_VAR_LIST:
            printf("VAR_LIST\n");
            printAST(node->var_list_data.var_list, indent+1);
            printAST(node->var_list_data.var, indent+1);
            break;

        case NODE_VAR:
            printf("VAR\n");
            printAST(node->var_data.id, indent+1);
            printAST(node->var_data.value, indent+1);
            break;

        case NODE_ID:
            printf("ID (name: %s)\n", node->id_data.sym->name);
            break;

        // case NODE_LITERAL:
        //     printf("Literal: ");
        //     if (node->literal_data.str_value) {
        //         printf("%s\n", node->literal_data.str_value);        // Print string literal
        //     } else {
        //         printf("%d\n", node->literal_data.int_value);        // Print integer literal
        //     }
        //     break;

        case NODE_OP:
            printf("Binary Operator: %s\n", node->op_data.op);
            printAST(node->op_data.left, indent + 1);                // Print the left operand
            printAST(node->op_data.right, indent + 1);               // Print the right operand
            break;

        case NODE_IF_ELSE:
            printf("If-Else Statement\n");
            printf("Condition:\n");
            printAST(node->if_else_data.condition, indent + 1);      // Print the condition
            printf("If Branch:\n");
            printAST(node->if_else_data.if_branch, indent + 1);      // Print the if branch
            if (node->if_else_data.else_branch) {
                printf("Else Branch:\n");
                printAST(node->if_else_data.else_branch, indent + 1); // Print the else branch, if it exists
            }
            break;

        case NODE_FOR:
            printf("For Loop\n");
            printf("Init:\n");
            printAST(node->for_data.init, indent + 1);               // Print the initialization statement
            printf("Condition:\n");
            printAST(node->for_data.condition, indent + 1);          // Print the loop condition
            printf("Increment:\n");
            printAST(node->for_data.increment, indent + 1);          // Print the increment statement
            printf("Body:\n");
            printAST(node->for_data.body, indent + 1);               // Print the loop body
            break;

        // case NODE_FUNC_DECL:
        //     printf("Function Declaration: %s\n", node->func_decl_data.sym->name);
        //     if (node->func_decl_data.params) {
        //         printf("Parameters:\n");
        //         for (int i = 0; i < node->child_count; i++) {
        //             printAST(node->func_decl_data.params[i], indent + 1);
        //         }
        //     }
        //     printf("Body:\n");
        //     printAST(node->func_decl_data.body, indent + 1);         // Print the function body
        //     break;

        case NODE_FUNC_CALL:
            printf("Function Call: %s\n", node->func_call_data.sym->name);
            if (node->func_call_data.args) {
                printf("Arguments:\n");
                for (int i = 0; i < node->func_call_data.arg_count; i++) {
                    printAST(node->func_call_data.args[i], indent + 1); // Print each argument
                }
            }
            break;

        // Handle other node types here...

        default:
            printf("Unknown Node Type\n");
    }
}


void freeAST(ASTNode* node) {
    if (node == NULL) return;

    // for (int i = 0; i < node->child_count; i++) {
    //     freeAST(node->children[i]);
    // }

    // free(node->children); // Free the children array
    free(node);          // Free the node itself
}

