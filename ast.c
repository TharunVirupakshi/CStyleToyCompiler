#include "symTable.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

extern int cur_line, cur_char;

// Function to create an empty AST node
ASTNode* createASTNode(NodeType type, int line_no, int char_no) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }

    node->type = type;
    node->line_no = line_no;
    node->char_no = char_no;
  
    return node;
}

const char* getDataTypeFromAST(ASTNode* node){
    const char* type = NULL;
    switch(node->type){
        case NODE_ID:
            type = node->id_data.sym->type;
            break;

        case NODE_ID_REF:
            if(node->id_ref_data.ref)
                type = node->id_ref_data.ref->type;
            break;
            
        case NODE_INT_LITERAL:
            type = "int";
            break;
            
        case NODE_CHAR_LITERAL:
            type = "char";
            break;
            
        case NODE_STR_LITERAL:
            type = "string";
            break;

        case NODE_EXPR_TERM:
            type = getDataTypeFromAST(node->expr_data.left);
            break;

        default:
            type = NULL;
            break;
    }

    return type;
}

void traverseAST(ASTNode* node, ASTTraversalCallback callback, void* context) {
    if (!node) return;

    // Call the callback function for the current node
    if(callback(node, context) == 0){
        // STOP TRAVERSAL
        return;
    }

    // Traverse based on node type
    switch (node->type) {
        case NODE_PROGRAM:
            traverseAST(node->program_data.stmt_list, callback, context);
            break;

        case NODE_RETURN:
            traverseAST(node->return_data.return_value, callback, context);
            break;

        case NODE_INT_LITERAL:
        case NODE_CHAR_LITERAL:
        case NODE_STR_LITERAL:
        case NODE_TYPE_SPEC:
        case NODE_ID:
        case NODE_ID_REF:
            // Leaf nodes
            break;

        case NODE_STMT_LIST:
            traverseAST(node->stmt_list_data.stmt_list, callback, context);
            traverseAST(node->stmt_list_data.stmt, callback, context);
            break;
        
        case NODE_STMT:
            traverseAST(node->stmt_data.stmt, callback, context);
            break;

        case NODE_BLOCK_STMT:
            traverseAST(node->block_stmt_data.stmt_list, callback, context);
            break;

        case NODE_DECL:
            traverseAST(node->decl_data.type_spec, callback, context);
            traverseAST(node->decl_data.var_list, callback, context);
            break;

        case NODE_VAR_LIST:
            traverseAST(node->var_list_data.var_list, callback, context);
            traverseAST(node->var_list_data.var, callback, context);
            break;

        case NODE_VAR:
            traverseAST(node->var_data.id, callback, context);
            traverseAST(node->var_data.value, callback, context);
            break;

        case NODE_ASSGN:
            printf("ASSGN\n");
            traverseAST(node->assgn_data.left, callback, context);
            traverseAST(node->assgn_data.right, callback, context);
            break;

        case NODE_EXPR_BINARY:
            traverseAST(node->expr_data.left, callback, context);
            traverseAST(node->expr_data.right, callback, context);
            break;

        case NODE_EXPR_UNARY:
            traverseAST(node->expr_data.left, callback, context);
            break;

        case NODE_EXPR_TERM:
            traverseAST(node->expr_data.left, callback, context);
            break;

        case NODE_IF:
            traverseAST(node->if_else_data.condition, callback, context);
            traverseAST(node->if_else_data.if_branch, callback, context);
            break;

        case NODE_IF_ELSE:
            traverseAST(node->if_else_data.condition, callback, context);
            traverseAST(node->if_else_data.if_branch, callback, context);
            traverseAST(node->if_else_data.else_branch, callback, context);
            break;

        case NODE_IF_COND:
            traverseAST(node->if_cond_data.cond, callback, context); 
            break;

        case NODE_IF_BRANCH:
        case NODE_ELSE_BRANCH:
            traverseAST(node->if_else_branch.branch, callback, context);;
            break;

        case NODE_WHILE:
            traverseAST(node->while_data.condition, callback, context);
            traverseAST(node->while_data.while_body, callback, context);
            break;
        case NODE_WHILE_COND:   
            traverseAST(node->while_cond_data.cond, callback, context);
            break;

        case NODE_WHILE_BODY:   
            traverseAST(node->while_body_data.body, callback, context);
            break;            

        case NODE_FOR:
            traverseAST(node->for_data.init, callback, context);
            traverseAST(node->for_data.condition, callback, context);
            traverseAST(node->for_data.updation, callback, context);
            traverseAST(node->for_data.body, callback, context);
            break;

        case NODE_FOR_INIT:
            traverseAST(node->for_init_data.init, callback, context);            
            break;

        case NODE_FOR_COND:
            traverseAST(node->for_cond_data.cond, callback, context); 
            break;

        case NODE_FOR_UPDATION:
            traverseAST(node->for_updation_data.updation, callback, context); 
            break;

        case NODE_FOR_BODY:
            traverseAST(node->for_body_data.body, callback, context); 
            break;

        case NODE_EXPR_COMMA_LIST:
            traverseAST(node->expr_comma_list_data.expr_comma_list, callback, context);
            traverseAST(node->expr_comma_list_data.expr_comma_list_item, callback, context);
            break;
        case NODE_FUNC_DECL:
            traverseAST(node->func_decl_data.id, callback, context);
            traverseAST(node->func_decl_data.params, callback, context);
            traverseAST(node->func_decl_data.body, callback, context);
            break;

        case NODE_FUNC_BODY:
            traverseAST(node->func_body_data.body, callback, context);
            break;

        case NODE_FUNC_CALL:
            traverseAST(node->func_call_data.id, callback, context);
            traverseAST(node->func_call_data.arg_list, callback, context);
            break;

        case NODE_PARAM_LIST:
            traverseAST(node->param_list_data.param_list, callback, context);
            traverseAST(node->param_list_data.param, callback, context);
            break;

        case NODE_ARG_LIST:
            traverseAST(node->arg_list_data.arg, callback, context);
            traverseAST(node->arg_list_data.arg_list, callback, context);
            break;

        case NODE_ARG:
            traverseAST(node->arg_data.arg, callback, context);
            break;

        default:
            break;
    }
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
            printAST(node->stmt_list_data.stmt, indent + 1, true);      // Print the individual statement
            printAST(node->stmt_list_data.stmt_list, indent + 1, false); // Print the statement list
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

int nodeCounter = 0;  // Unique ID counter for each node

// Helper function to generate unique node IDs
int generateNodeID() {
    return nodeCounter++;
}





#define INITIAL_EDGE_BUFFER_SIZE 1024

// Helper function to export AST node as JSON
void exportASTNodeAsJSON(FILE *file, ASTNode *node, int parentID, int *edgeBufferSize, char **edgeBuffer, int isFirstNode) {
    if (!node) return;

    int currentID = generateNodeID();

    // Add current node, ensuring proper formatting (comma-separated) based on whether it's the first node or not
    if (!isFirstNode) {
        fprintf(file, ", ");
    }
    fprintf(file, "{ \"id\": %d, \"label\": \"", currentID);

    // Handle different node types
    switch (node->type) {
        case NODE_PROGRAM:
            fprintf(file, "PROGRAM\" }");
            break;
        case NODE_RETURN:
            fprintf(file, "RETURN\" }");
            break;
        case NODE_INT_LITERAL:
            fprintf(file, "LITERAL (int: %d)\" }", node->literal_data.value.int_value);
            break;
        case NODE_CHAR_LITERAL:
            fprintf(file, "LITERAL (char: '%c')\" }", node->literal_data.value.char_value);
            break;
        case NODE_STR_LITERAL:
            fprintf(file, "LITERAL (string)\" }");
            break;
        case NODE_STMT_LIST:
            fprintf(file, "STMT LIST\" }");
            break;
        case NODE_STMT:
            fprintf(file, "STMT\" }");
            break;
        case NODE_BLOCK_STMT:
            fprintf(file, "BLOCK STMT\" }");
            break;
        case NODE_DECL:
            fprintf(file, "DECL\" }");
            break;
        case NODE_TYPE_SPEC:
            fprintf(file, "TYPE_SPEC (%s)\" }", node->type_data.type);
            break;
        case NODE_VAR_LIST:
            fprintf(file, "VAR_LIST\" }");
            break;
        case NODE_VAR:
            fprintf(file, "VAR (name: %s)\" }", node->var_data.id->id_data.sym->name);
            break;
        case NODE_ID:
            fprintf(file, "ID (name: %s)\" }", node->id_data.sym->name);
            break;
        case NODE_ID_REF:
            fprintf(file, "ID_REF (name: %s)\" }", node->id_ref_data.name);
            break;
        case NODE_ASSGN:
            fprintf(file, "ASSGN\" }");
            break;
        case NODE_EXPR_BINARY:
            fprintf(file, "EXPR (binary: %s)\" }", node->expr_data.op);
            break;
        case NODE_EXPR_UNARY:
            fprintf(file, "EXPR (unary: %s)\" }", node->expr_data.op);
            break;
        case NODE_EXPR_TERM:
            fprintf(file, "EXPR (term)\" }");
            break;
        case NODE_IF:
            fprintf(file, "IF\" }");
            break;
        case NODE_IF_ELSE:
            fprintf(file, "IF ELSE\" }");
            break;
        case NODE_IF_COND:
            fprintf(file, "IF_COND\" }");
            break; 
        case NODE_IF_BRANCH:
            fprintf(file, "IF_BRANCH\" }");
            break; 
        case NODE_ELSE_BRANCH:
            fprintf(file, "ELSE_BRANCH\" }");
            break; 
        case NODE_WHILE:
            fprintf(file, "WHILE\" }");
            break;
        case NODE_WHILE_COND:
            fprintf(file, "WHILE_COND\" }");
            break;
        case NODE_WHILE_BODY:
            fprintf(file, "WHILE_BODY\" }");
            break;
        case NODE_FOR:
            fprintf(file, "FOR\" }");
            break;
        case NODE_FOR_INIT:
            fprintf(file, "FOR_INIT\" }");
            break;
        case NODE_FOR_COND:
            fprintf(file, "FOR_COND\" }");
            break;
        case NODE_FOR_UPDATION:
            fprintf(file, "FOR_UPDATION\" }");
            break;
        case NODE_EXPR_COMMA_LIST:
            fprintf(file, "EXPR_COMMA_LIST\" }");
            break; 
        case NODE_FOR_BODY:
            fprintf(file, "FOR_BODY\" }");
            break;
        case NODE_FUNC_DECL:
            fprintf(file, "FUNC_DECL (name: %s)\" }", node->func_decl_data.id->id_data.sym->name);
            break;
        case NODE_FUNC_BODY:
            fprintf(file, "FUNC_BODY\" }");
            break;
        case NODE_PARAM_LIST:
            fprintf(file, "PARAM_LIST\" }");
            break;
        case NODE_PARAM:
            fprintf(file, "PARAM (type: %s)\" }", node->param_data.type_spec->type_data.type);
            break;
        case NODE_FUNC_CALL:
            fprintf(file, "FUNC_CALL (name: %s)\" }", node->func_call_data.id->id_ref_data.name);
            break;
        case NODE_ARG_LIST:
            fprintf(file, "ARG_LIST\" }");
            break;
        case NODE_ARG:
            fprintf(file, "ARG\" }");
            break;
        default:
            fprintf(file, "UNKNOWN\" }");
            break;
    }

    // Store edge in the buffer if there's a parent
    if (parentID != -1) {
        // Resize buffer if needed
        if (*edgeBufferSize <= 0) {
            *edgeBufferSize = INITIAL_EDGE_BUFFER_SIZE;
            *edgeBuffer = malloc(*edgeBufferSize);
            (*edgeBuffer)[0] = '\0'; // Initialize the buffer
        }

        // Prepare the edge representation
        char edge[64]; // Temporary buffer for the edge
        snprintf(edge, sizeof(edge), "{ \"from\": %d, \"to\": %d }, ", parentID, currentID);

        // Check if the buffer needs to be resized
        if (strlen(*edgeBuffer) + strlen(edge) >= *edgeBufferSize) {
            *edgeBufferSize *= 2; // Double the size
            *edgeBuffer = realloc(*edgeBuffer, *edgeBufferSize);
        }

        strcat(*edgeBuffer, edge);
    }

    // Recurse for child nodes
    switch (node->type) {
        case NODE_PROGRAM:
            exportASTNodeAsJSON(file, node->program_data.stmt_list, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_RETURN:
            exportASTNodeAsJSON(file, node->return_data.return_value, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_STMT_LIST:
            exportASTNodeAsJSON(file, node->stmt_list_data.stmt_list, currentID, edgeBufferSize, edgeBuffer, 0);
            exportASTNodeAsJSON(file, node->stmt_list_data.stmt, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_DECL:
            exportASTNodeAsJSON(file, node->decl_data.type_spec, currentID, edgeBufferSize, edgeBuffer, 0);
            exportASTNodeAsJSON(file, node->decl_data.var_list, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_VAR_LIST:
            exportASTNodeAsJSON(file, node->var_list_data.var_list, currentID, edgeBufferSize, edgeBuffer, 0);
            exportASTNodeAsJSON(file, node->var_list_data.var, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_VAR:
            exportASTNodeAsJSON(file, node->var_data.id, currentID, edgeBufferSize, edgeBuffer, 0);
            exportASTNodeAsJSON(file, node->var_data.value, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_ASSGN:
            exportASTNodeAsJSON(file, node->assgn_data.left, currentID, edgeBufferSize, edgeBuffer, 0);
            exportASTNodeAsJSON(file, node->assgn_data.right, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_EXPR_BINARY:
            exportASTNodeAsJSON(file, node->expr_data.left, currentID, edgeBufferSize, edgeBuffer, 0);
            exportASTNodeAsJSON(file, node->expr_data.right, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_EXPR_UNARY:
            exportASTNodeAsJSON(file, node->expr_data.left, currentID, edgeBufferSize, edgeBuffer, 0);
            break;

        case NODE_EXPR_TERM:
            exportASTNodeAsJSON(file, node->expr_data.left, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        
        case NODE_IF:
            exportASTNodeAsJSON(file, node->if_else_data.condition, currentID, edgeBufferSize, edgeBuffer, 0);
            exportASTNodeAsJSON(file, node->if_else_data.if_branch, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_IF_ELSE:
            exportASTNodeAsJSON(file, node->if_else_data.condition, currentID, edgeBufferSize, edgeBuffer, 0);
            exportASTNodeAsJSON(file, node->if_else_data.if_branch, currentID, edgeBufferSize, edgeBuffer, 0);
            exportASTNodeAsJSON(file, node->if_else_data.else_branch, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_IF_COND:
            exportASTNodeAsJSON(file, node->if_cond_data.cond, currentID, edgeBufferSize, edgeBuffer, 0);
            break;        
        case NODE_IF_BRANCH:
            exportASTNodeAsJSON(file, node->if_else_branch.branch, currentID, edgeBufferSize, edgeBuffer, 0);
            break;        
        case NODE_ELSE_BRANCH:
            exportASTNodeAsJSON(file, node->if_else_branch.branch, currentID, edgeBufferSize, edgeBuffer, 0);
            break;        
        


        case NODE_WHILE:
            exportASTNodeAsJSON(file, node->while_data.condition, currentID, edgeBufferSize, edgeBuffer, 0);
            exportASTNodeAsJSON(file, node->while_data.while_body, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_WHILE_COND:
            exportASTNodeAsJSON(file, node->while_cond_data.cond, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_WHILE_BODY:
            exportASTNodeAsJSON(file, node->while_body_data.body, currentID, edgeBufferSize, edgeBuffer, 0);
            break;

        case NODE_FOR:
            exportASTNodeAsJSON(file, node->for_data.init, currentID, edgeBufferSize, edgeBuffer, 0);
            exportASTNodeAsJSON(file, node->for_data.condition, currentID, edgeBufferSize, edgeBuffer, 0);
            exportASTNodeAsJSON(file, node->for_data.updation, currentID, edgeBufferSize, edgeBuffer, 0);
            exportASTNodeAsJSON(file, node->for_data.body, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_FOR_INIT:
            exportASTNodeAsJSON(file, node->for_init_data.init, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_FOR_COND:
            exportASTNodeAsJSON(file, node->for_cond_data.cond, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_EXPR_COMMA_LIST:
            exportASTNodeAsJSON(file, node->expr_comma_list_data.expr_comma_list, currentID, edgeBufferSize, edgeBuffer, 0);
            exportASTNodeAsJSON(file, node->expr_comma_list_data.expr_comma_list_item, currentID, edgeBufferSize, edgeBuffer, 0);
            break;            
        case NODE_FOR_UPDATION:
            exportASTNodeAsJSON(file, node->for_updation_data.updation, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_FOR_BODY:
            exportASTNodeAsJSON(file, node->for_body_data.body, currentID, edgeBufferSize, edgeBuffer, 0);
            break;

        case NODE_FUNC_DECL:
            exportASTNodeAsJSON(file, node->func_decl_data.id, currentID, edgeBufferSize, edgeBuffer, 0);
            exportASTNodeAsJSON(file, node->func_decl_data.params, currentID, edgeBufferSize, edgeBuffer, 0);
            exportASTNodeAsJSON(file, node->func_decl_data.body, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_FUNC_BODY:
            exportASTNodeAsJSON(file, node->func_body_data.body, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_BLOCK_STMT:
            exportASTNodeAsJSON(file, node->block_stmt_data.stmt_list, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_PARAM_LIST:
            exportASTNodeAsJSON(file, node->param_list_data.param_list, currentID, edgeBufferSize, edgeBuffer, 0);
            exportASTNodeAsJSON(file, node->param_list_data.param, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_PARAM:
            exportASTNodeAsJSON(file, node->param_data.id, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_FUNC_CALL:
            exportASTNodeAsJSON(file, node->func_call_data.arg_list, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_ARG_LIST:
            exportASTNodeAsJSON(file, node->arg_list_data.arg, currentID, edgeBufferSize, edgeBuffer, 0);
            exportASTNodeAsJSON(file, node->arg_list_data.arg_list, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        case NODE_ARG:
            exportASTNodeAsJSON(file, node->arg_data.arg, currentID, edgeBufferSize, edgeBuffer, 0);
            break;
        default:
            break;
    }
}

// Function to create directory if it doesn't exist
int createDirectory(const char *folderPath){
    #ifdef _WIN32
    if (_mkdir(folderPath) == -1) {
    #else
    if (mkdir(folderPath, 0777) == -1) {
    #endif
    
        if (errno != EEXIST) {
            perror("Error creating directory");
            return 1;
        }
    }
    return 0;
}


// Export the entire AST as a JSON structure
void exportASTAsJSON(const char *folderPath, ASTNode *root) {


    // Construct the full path for ast.json
    char filePath[256];
    snprintf(filePath, sizeof(filePath), "%s/ast.json", folderPath);
    
    createDirectory(folderPath);

    FILE *file = fopen(filePath, "w");
    
    

    fprintf(file, "{ \"nodes\": [");
    
    int edgeBufferSize = 0;
    char *edgeBuffer = NULL;
    
    exportASTNodeAsJSON(file, root, -1, &edgeBufferSize, &edgeBuffer, 1); // isFirstNode flag set to 1 for the root node

    // Clean up the edge buffer by removing the last comma and space
    if (edgeBuffer && strlen(edgeBuffer) > 0) {
        edgeBuffer[strlen(edgeBuffer) - 2] = '\0'; // Remove last ", "
    }

    fprintf(file, "], \"edges\": [%s]}", edgeBuffer ? edgeBuffer : ""); // Insert the collected edges here

    // Generate the index.html file
    snprintf(filePath, sizeof(filePath), "%s/index.html", folderPath);
    FILE *htmlFile = fopen(filePath, "w");

    if (htmlFile) {
        fprintf(htmlFile, "<!DOCTYPE html>\n");
        fprintf(htmlFile, "<html lang=\"en\">\n");
        fprintf(htmlFile, "<head>\n");
        fprintf(htmlFile, "    <meta charset=\"UTF-8\">\n");
        fprintf(htmlFile, "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
        fprintf(htmlFile, "    <title>AST Graph Display</title>\n");
        fprintf(htmlFile, "    <script src=\"https://unpkg.com/vis-network/standalone/umd/vis-network.min.js\"></script>\n");
        fprintf(htmlFile, "    <style>\n");
        fprintf(htmlFile, "        #mynetwork {\n");
        fprintf(htmlFile, "            width: 100vw;\n");
        fprintf(htmlFile, "            height: 100vh;\n");
        fprintf(htmlFile, "            border: 1px solid lightgray;\n");
        fprintf(htmlFile, "        }\n");
        fprintf(htmlFile, "        body {\n");
        fprintf(htmlFile, "            padding: 0;\n");
        fprintf(htmlFile, "            margin: 0;\n");
        fprintf(htmlFile, "            font-family: 'Courier New', Courier, monospace;\n");
        fprintf(htmlFile, "        }\n");
        fprintf(htmlFile, "    </style>\n");
        fprintf(htmlFile, "</head>\n");
        fprintf(htmlFile, "<body>\n");
        fprintf(htmlFile, "    <h1 style=\"text-align:center\">Abstract Syntax Tree (AST) Graph</h1>\n");
        fprintf(htmlFile, "    <div id=\"mynetwork\"></div>\n");
        fprintf(htmlFile, "    <script>\n");
        fprintf(htmlFile, "        fetch(\"ast.json\")\n");
        fprintf(htmlFile, "            .then(response => response.json())\n");
        fprintf(htmlFile, "            .then(data => {\n");
        fprintf(htmlFile, "                const nodes = new vis.DataSet(data.nodes);\n");
        fprintf(htmlFile, "                const edges = new vis.DataSet(data.edges);\n");
        fprintf(htmlFile, "                const container = document.getElementById('mynetwork');\n");
        fprintf(htmlFile, "                const dataSet = { nodes: nodes, edges: edges };\n");
        fprintf(htmlFile, "                const options = {\n");
        fprintf(htmlFile, "                    nodes: { shape: 'dot', size: 10 },\n");
        fprintf(htmlFile, "                    physics: { enabled: true ,hierarchicalRepulsion: {nodeDistance: 150}},\n");
        fprintf(htmlFile, "                    layout: { hierarchical: { parentCentralization: true, shakeTowards: 'roots', direction: 'UD', sortMethod: 'directed', nodeSpacing: 150 } }\n");
        fprintf(htmlFile, "                };\n");
        fprintf(htmlFile, "                const network = new vis.Network(container, dataSet, options);\n");
        fprintf(htmlFile, "            })\n");
        fprintf(htmlFile, "            .catch(error => console.error('Error loading the JSON data:', error));\n");
        fprintf(htmlFile, "    </script>\n");
        fprintf(htmlFile, "</body>\n");
        fprintf(htmlFile, "</html>\n");
        fclose(htmlFile);
    } else {
        printf("Error: Unable to open index.html for writing.\n");
    }

    // Free the edge buffer if it was allocated
    free(edgeBuffer);

    printf("Exported AST to %s/ast.json\n", folderPath);
}

