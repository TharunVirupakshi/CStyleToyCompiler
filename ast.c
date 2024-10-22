#include "symTable.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


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
void printAST(ASTNode* node, int indent, bool isLast) {
    if (node == NULL) return;

    // Print indentation, vertical lines, and dashes
    for (int i = 0; i < indent - 1; i++) {
        printf("|   ");
    }

    if (indent > 0) {
        if (isLast) {
            printf("└── ");
        } else {
            printf("├── ");
        }
    }

    switch (node->type) {
        case NODE_PROGRAM:
            printf("PROGRAM\n");
            printAST(node->program_data.stmt_list, indent + 1, true);
            break;

        case NODE_RETURN:
            printf("RETURN\n");
            printAST(node->return_data.return_value, indent + 1, true); // Recursively print the return value
            break;

        case NODE_INT_LITERAL:
            printf("LITERAL (type: Int, value: %d)\n", node->literal_data.value.int_value);
            break;

        case NODE_CHAR_LITERAL:
            printf("LITERAL (type: Char, value: %c)\n", node->literal_data.value.char_value);
            break;

        case NODE_STR_LITERAL:
            printf("LITERAL (type: String, value: %s)\n", node->literal_data.value.str_value);
            break;

        case NODE_STMT_LIST:
            printf("STMT LIST\n");
            printAST(node->stmt_list_data.stmt_list, indent + 1, false); // Print the statement list
            printAST(node->stmt_list_data.stmt, indent + 1, true);      // Print the individual statement
            break;

        case NODE_STMT:
            printf("STMT\n");
            printAST(node->stmt_data.stmt, indent + 1, true);          // Recursively print the statement
            break;

        case NODE_BLOCK_STMT:
            printf("BLOCK STMT\n");
            printAST(node->block_stmt_data.stmt_list, indent + 1, true);
            break;

        case NODE_DECL:
            printf("DECL\n");
            printAST(node->decl_data.var_list, indent + 1, false);
            printAST(node->decl_data.type_spec, indent + 1, true);
            break;

        case NODE_TYPE_SPEC:
            printf("TYPE_SPEC (type: %s)\n", node->type_data.type);
            break;

        case NODE_VAR_LIST:
            printf("VAR_LIST\n");
            printAST(node->var_list_data.var, indent + 1, false);
            printAST(node->var_list_data.var_list, indent + 1, false);
            break;

        case NODE_VAR:
            printf("VAR\n");
            printAST(node->var_data.value, indent + 1, false);
            printAST(node->var_data.id, indent + 1, true);
            break;

        case NODE_ID:
            printf("ID (name: %s)\n", node->id_data.sym->name);
            break;

        case NODE_ID_REF:
            printf("ID REF (name: %s)\n", node->id_ref_data.name);
            break;

        case NODE_EXPR_BINARY:
            printf("EXPR (op: %s)\n", node->expr_data.op);
            printAST(node->expr_data.left, indent + 1, false);
            printAST(node->expr_data.right, indent + 1, true);
            break;

        case NODE_EXPR_UNARY:
            printf("EXPR (op: %s)\n", node->expr_data.op);
            printAST(node->expr_data.left, indent + 1, true);
            break;

        case NODE_EXPR_TERM:
            printf("EXPR (term)\n");
            printAST(node->expr_data.left, indent + 1, true);
            break;

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

