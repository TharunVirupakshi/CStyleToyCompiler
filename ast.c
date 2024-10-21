#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



// Function to create an empty AST node
ASTNode* createASTNode(NodeType type, int childCount) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }

    node->type = type;
    node->children = (ASTNode**)malloc(sizeof(ASTNode*) * childCount);
    if (!node->children) {
        fprintf(stderr, "Memory allocation error\n");
        free(node);
        exit(1);
    }

    node->child_count = childCount;
    for (int i = 0; i < childCount; i++) {
        node->children[i] = NULL;
    }


    return node;
}



// // Function to free the AST
// void freeAST(ASTNode* node) {
//     if (node == NULL) {
//         return;
//     }

//     // Recursively free all child nodes
//     for (int i = 0; i < node->childCount; i++) {
//         freeAST(node->children[i]);
//     }

//     // Free any dynamically allocated values, depending on the node type
//     switch (node->type) {
//         case NODE_ID:
//         case NODE_STR_LITERAL:
//             free(node->strValue);  // Free the string value
//             break;
//         // If other types had dynamically allocated values, handle them here as well
//         default:
//             break;
//     }

//     // Free the array of children (if any)
//     if (node->children != NULL) {
//         free(node->children);
//     }

//     // Finally, free the node itself
//     free(node);
// }
