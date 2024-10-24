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

        case NODE_ASSGN:
            printf("ASSGN\n");
            printAST(node->assgn_data.right, indent+1, true);
            printAST(node->assgn_data.left, indent+1, true);
            break;

        case NODE_EXPR_BINARY:
            printf("EXPR (op: %s)\n", node->expr_data.op);
            printAST(node->expr_data.right, indent + 1, true);
            printAST(node->expr_data.left, indent + 1, false);
            break;

        case NODE_EXPR_UNARY:
            printf("EXPR (op: %s)\n", node->expr_data.op);
            printAST(node->expr_data.left, indent + 1, true);
            break;

        case NODE_EXPR_TERM:
            printf("EXPR (term)\n");
            printAST(node->expr_data.left, indent + 1, true);
            break;

        case NODE_IF:
            printf("IF\n");
            printAST(node->if_else_data.if_branch, indent+1, true);
            printAST(node->if_else_data.condition, indent+1, true);
            break;

        case NODE_IF_ELSE:
            printf("IF ELSE\n");
            printAST(node->if_else_data.else_branch, indent+1, true);
            printAST(node->if_else_data.if_branch, indent+1, true);
            printAST(node->if_else_data.condition, indent+1, true);
            break;
        
        case NODE_IF_COND:
            printf("IF COND\n");
            printAST(node->if_cond_data.cond, indent+1, true);
            break;

        case NODE_IF_BRANCH:
            printf("IF BRANCH\n");
            printAST(node->if_else_branch.branch, indent+1, true);
            break;

        case NODE_ELSE_BRANCH:
            printf("ELSE BRANCH\n");
            printAST(node->if_else_branch.branch, indent+1, true);
            break;

        case NODE_WHILE:
            printf("WHILE\n");
            printAST(node->while_data.while_body, indent+1, true);
            printAST(node->while_data.condition, indent+1, true);
            break;

        case NODE_WHILE_COND:   
            printf("WHILE COND\n");
            printAST(node->while_cond_data.cond, indent+1, true);
            break;

        case NODE_WHILE_BODY:   
            printf("WHILE BODY\n");
            printAST(node->while_body_data.body, indent+1, true);
            break;

        case NODE_FOR:
            printf("FOR\n");
            printAST(node->for_data.body, indent+1, true);
            printAST(node->for_data.updation, indent+1, true);
            printAST(node->for_data.condition, indent+1, true);
            printAST(node->for_data.init, indent+1, true);
            break;

        case NODE_FOR_INIT:
            printf("FOR_INIT\n");
            printAST(node->for_init_data.init, indent+1, true);
            break;

        case NODE_FOR_COND:
            printf("FOR_COND\n");
            printAST(node->for_cond_data.cond, indent+1, true); 
            break;

        case NODE_FOR_UPDATION:
            printf("FOR_UPDATION\n");
            printAST(node->for_updation_data.updation, indent+1, true); 
            break;

        case NODE_FOR_BODY:
            printf("FOR_BODY\n");
            printAST(node->for_body_data.body, indent+1, true); 
            break; 

        case NODE_EXPR_COMMA_LIST:
            printf("EXPR COMMA LIST\n");
            printAST(node->expr_comma_list_data.expr_comma_list_item, indent+1, true);
            printAST(node->expr_comma_list_data.expr_comma_list, indent+1, true);
            break;

        case NODE_FUNC_DECL:
            printf("FUNC DECL (type: %s, name: %s, params: %d)\n",
            node->func_decl_data.id->id_data.sym->type, 
            node->func_decl_data.id->id_data.sym->name,
            node->func_decl_data.param_count);

            printAST(node->func_decl_data.body, indent+1, true);
            printAST(node->func_decl_data.params, indent+1, true);
            printAST(node->func_decl_data.id, indent+1, true);
            break;

        case NODE_FUNC_BODY:
            printf("FUNC BODY\n");
            printAST(node->func_body_data.body, indent+1, true);
            break;

        case NODE_PARAM_LIST:
            printf("PARAM LIST\n");
            printAST(node->param_list_data.param, indent+1, true);
            printAST(node->param_list_data.param_list, indent+1, true);
            break;

        case NODE_PARAM:
            printf("PARAM (type: %s)\n", node->param_data.type_spec->type_data.type);
            printAST(node->param_data.id, indent+1, true);
            break;

        case NODE_FUNC_CALL:
            printf("FUNC CALL (name: %s, arg_count: %d)\n", 
            node->func_call_data.id->id_ref_data.name,
            node->func_call_data.arg_count);
            printAST(node->func_call_data.arg_list, indent+1, false);
            printAST(node->func_call_data.id, indent+1, true);
            break;

        case NODE_ARG_LIST:
            printf("ARG_LIST\n");
            printAST(node->arg_list_data.arg, indent+1, false);
            printAST(node->arg_list_data.arg_list, indent+1, true);
            break;

        case NODE_ARG:
            printf("ARG\n");
            printAST(node->arg_data.arg, indent+1, true);
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

