#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printErrors();

// Error structure
typedef struct Error {
    char* message;
    int line;
    int char_no;
} Error;

extern bool isSemanticError;

// Error list
Error* errorList = NULL;
int errorCount = 0;

// Main function
SemanticStatus performSemanticAnalysis(ASTNode* root, SymbolTable* globalTable) {
    if (!root) return SEMANTIC_ERROR;
    checkDuplicates(globalTable);
    validateSymbolUsage(root);

    // print errors if any
    if(errorList){
        printErrors();
    } 

    return errorCount > 0 ? SEMANTIC_ERROR : SEMANTIC_SUCCESS;
}





// Add an error to the list
void addError(const char* message, int line, int char_no) {
    errorList = realloc(errorList, (errorCount + 1) * sizeof(Error));
    if (!errorList) {
        fprintf(stderr, "Memory allocation failed for error list\n");
        exit(1);
    }
    errorList[errorCount].message = strdup(message);
    errorList[errorCount].line = line;
    errorList[errorCount].char_no = char_no;
    errorCount++;
}

// Function to print all stored errors
void printErrors() {
    for (int i = 0; i < errorCount; i++) {
        printf("Error: %s at line %d, char %d\n", errorList[i].message, errorList[i].line, errorList[i].char_no);
        free(errorList[i].message); // Free each error message after printing
    }
    free(errorList); // Free the entire list at the end
    errorList = NULL; // Reset the list pointer
 
}

// Modified checkDuplicates function
void checkDuplicates(SymbolTable* table) {
    if (!table) return;

    for (int i = 0; i < table->size; ++i) {
        if(table->symbols[i]->is_duplicate) continue;

        for (int j = i+1; j < table->size; ++j) {
            
            if (strcmp(table->symbols[i]->name, table->symbols[j]->name) == 0) {
                table->symbols[j]->is_duplicate = 1;
                char errorMsg[256];
                snprintf(errorMsg, sizeof(errorMsg), "Duplicate declaration of '%s' (previously declared at line %d, char %d)",
                    table->symbols[j]->name,
                    table->symbols[i]->line_no,
                    table->symbols[i]->char_no);
                addError(errorMsg, table->symbols[j]->line_no, table->symbols[j]->char_no);
            }
        }
    }

    // Check child scopes recursively
    for (int k = 0; k < table->num_children; ++k) {
        checkDuplicates(table->children[k]);
    }
}

void validateSymbolUsage(ASTNode* root){
    if (!root) return;

    // If the node is an identifier (e.g., variable reference)
    if (root->type == NODE_ID_REF) {
        const char* varName = root->id_ref_data.name;  // Assuming the ID node stores the name in `value.id`
        
        // Look up the symbol in the current or any parent scope
        SymbolTable* currentScope = root->id_ref_data.scope;
        symbol* foundSymbol = lookupSymbol(currentScope, varName);

        // If the symbol is not found, it's an undeclared variable
        if (!foundSymbol) {
            char errorMsg[256];
            snprintf(errorMsg, sizeof(errorMsg), "Undeclared variable '%s'", varName);
            addError(errorMsg, root->id_ref_data.line_no, root->id_ref_data.char_no);  // Adding error with line and char info
        }else{
            // Bind the id_ref node to sym

            root->id_ref_data.ref = foundSymbol;
        }
    }

    // Traverse the children
    // Traverse each type of node according to the specific structure
    switch (root->type) {
        case NODE_PROGRAM:
            validateSymbolUsage(root->program_data.stmt_list);
            break;

        case NODE_RETURN:
            validateSymbolUsage(root->return_data.return_value);
            break;

        case NODE_STMT_LIST:
            validateSymbolUsage(root->stmt_list_data.stmt_list);
            validateSymbolUsage(root->stmt_list_data.stmt);
            break;

        case NODE_STMT:
            validateSymbolUsage(root->stmt_data.stmt);
            break;

        case NODE_BLOCK_STMT:
            validateSymbolUsage(root->block_stmt_data.stmt_list);
            break;

        case NODE_DECL:
            // validateSymbolUsage(root->decl_data.type_spec);
            validateSymbolUsage(root->decl_data.var_list);
            break;

        case NODE_VAR_LIST:
            validateSymbolUsage(root->var_list_data.var_list);
            validateSymbolUsage(root->var_list_data.var);
            break;

        case NODE_VAR:
            validateSymbolUsage(root->var_data.id);
            validateSymbolUsage(root->var_data.value);
            break;

        case NODE_ASSGN:
            validateSymbolUsage(root->assgn_data.left);
            validateSymbolUsage(root->assgn_data.right);
            break;

        case NODE_EXPR_BINARY:
            validateSymbolUsage(root->expr_data.left);
            validateSymbolUsage(root->expr_data.right);
            break;

        case NODE_EXPR_UNARY:
        case NODE_EXPR_TERM:
            validateSymbolUsage(root->expr_data.left);
            break;

        case NODE_IF:
            validateSymbolUsage(root->if_else_data.condition);
            validateSymbolUsage(root->if_else_data.if_branch);
            break;

        case NODE_IF_ELSE:
            validateSymbolUsage(root->if_else_data.condition);
            validateSymbolUsage(root->if_else_data.if_branch);
            validateSymbolUsage(root->if_else_data.else_branch);
            break;

        case NODE_IF_COND:
            validateSymbolUsage(root->if_cond_data.cond);
            break;
        
        case NODE_IF_BRANCH:
        case NODE_ELSE_BRANCH:
            validateSymbolUsage(root->if_else_branch.branch);
            break;
        

        case NODE_WHILE:
            validateSymbolUsage(root->while_data.condition);
            validateSymbolUsage(root->while_data.while_body);
            break;

        case NODE_WHILE_COND:   
            validateSymbolUsage(root->while_cond_data.cond);
            break;

        case NODE_WHILE_BODY:   
            validateSymbolUsage(root->while_body_data.body);
            break;

        case NODE_FOR:
            validateSymbolUsage(root->for_data.init);
            validateSymbolUsage(root->for_data.condition);
            validateSymbolUsage(root->for_data.updation);
            validateSymbolUsage(root->for_data.body);
            break;

        case NODE_FOR_INIT:
            validateSymbolUsage(root->for_init_data.init);
            break;

        case NODE_FOR_COND: 
            validateSymbolUsage(root->for_cond_data.cond); 
            break;

        case NODE_FOR_UPDATION: 
            validateSymbolUsage(root->for_updation_data.updation); 
            break;

        case NODE_FOR_BODY: 
            validateSymbolUsage(root->for_body_data.body); 
            break; 

        case NODE_EXPR_COMMA_LIST:
            validateSymbolUsage(root->expr_comma_list_data.expr_comma_list);
            validateSymbolUsage(root->expr_comma_list_data.expr_comma_list_item);
            break;
        

        case NODE_FUNC_DECL:
            validateSymbolUsage(root->func_decl_data.params);
            validateSymbolUsage(root->func_decl_data.body);
            break;
        
        case NODE_FUNC_BODY:
            validateSymbolUsage(root->func_body_data.body);
            break;

        case NODE_FUNC_CALL:
            validateSymbolUsage(root->func_call_data.id);
            validateSymbolUsage(root->func_call_data.arg_list);
            break;

        // case NODE_PARAM_LIST:
        //     validateSymbolUsage(root->param_list_data.param);
        //     validateSymbolUsage(root->param_list_data.param_list);
        //     break;

        case NODE_ARG_LIST:
            validateSymbolUsage(root->arg_list_data.arg_list);
            validateSymbolUsage(root->arg_list_data.arg);
            break;

        case NODE_ARG:
            validateSymbolUsage(root->arg_data.arg);
            break;

        default:
            break;
    }
}


