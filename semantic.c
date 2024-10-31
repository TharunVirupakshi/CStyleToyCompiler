#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TYPE_VOID "void"
#define TYPE_INT "int"
#define TYPE_CHAR "char"
#define TYPE_STRING "string"
#define TYPE_UNKNOWN "unknown"

typedef enum{
    OP_ARITHMETIC,
    OP_COMP,
    OP_LOGICAL,
    OP_INC_DEC,
    OP_UNKNOWN
}OpType;


OpType getOpType(const char* op) {
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 ||
        strcmp(op, "*") == 0 || strcmp(op, "/") == 0){
        return OP_ARITHMETIC;
    } 
    else if (strcmp(op, "==") == 0 || strcmp(op, "<=") == 0 ||
             strcmp(op, ">=") == 0 || strcmp(op, "<") == 0 ||
             strcmp(op, ">") == 0 || strcmp(op, "!=") == 0) {
        return OP_COMP;
    }
    else if (strcmp(op, "&&") == 0 || strcmp(op, "||") == 0 || strcmp(op, "!") == 0) {
        return OP_LOGICAL;
    }
    else if (strcmp(op, "PRE_INC") == 0 || strcmp(op, "PRE_DEC") == 0 ||
             strcmp(op, "POST_INC") == 0 || strcmp(op, "POST_DEC") == 0) {
        return OP_INC_DEC;
    }
    
    return OP_UNKNOWN;  // For unsupported or unknown operators
}

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
    validateTypes(root);
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
        printf("Error: %s at (line %d, char %d)\n", errorList[i].message, errorList[i].line, errorList[i].char_no);
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
            addError(errorMsg, root->line_no, root->char_no);  // Adding error with line and char info
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



const char* promoteType(const char* leftType, const char* rightType) {
    if (!leftType || !rightType) return NULL;

    // Both types match exactly, no promotion needed
    if (strcmp(leftType, rightType) == 0) return leftType;

    // Promote char to int when paired with int
    if ((strcmp(leftType, TYPE_INT) == 0 && strcmp(rightType, TYPE_CHAR) == 0) ||
        (strcmp(leftType, TYPE_CHAR) == 0 && strcmp(rightType, TYPE_INT) == 0)) {
        return TYPE_INT;
    }
    

    // No valid promotion path
    return NULL;
}

const char* inferAndValidateType(ASTNode* node) {

    const char* type = NULL;

    if (!node) return NULL;
    
    switch (node->type) {
        case NODE_ID:
            // printf("Getting type of ID\n");
            type = node->id_data.sym->type;
            if(!type){
                char errorMsg[256]; 
                snprintf(errorMsg, sizeof(errorMsg),"Type of '%s' is NULL", node->id_data.sym->name);
                addError(errorMsg, node->line_no, node->char_no);
            } 
            node->inferedType = type;
            break;

        case NODE_ID_REF:
            // printf("Getting type of ID ref\n");
            type = node->id_ref_data.ref ? node->id_ref_data.ref->type : NULL;
            
            if(!type){
                char errorMsg[256]; 
                snprintf(errorMsg, sizeof(errorMsg),"Type of '%s' is NULL", node->id_ref_data.name);
                addError(errorMsg, node->line_no, node->char_no);
                return NULL;
            } 
            node->inferedType = type;            
            break;
        case NODE_INT_LITERAL:
            // printf("Getting type of int\n");
            type = "int";
            node->inferedType = type;
            break;
        case NODE_CHAR_LITERAL:
            // printf("Getting type of char\n");
            type = "char";
            node->inferedType = type;
            break;
        case NODE_STR_LITERAL:
            type = "string";
            node->inferedType = type;
            break;
        case NODE_EXPR_TERM:
            // printf("Getting type of expr term\n");
            type = inferAndValidateType(node->expr_data.left);
            node->inferedType = type;

            break;
        case NODE_FUNC_CALL:
            type = inferAndValidateType(node->func_call_data.id);
            node->inferedType = type;
            break;

        case NODE_VAR:
            {   
                if(!node->var_data.value) break;

                const char* leftType = inferAndValidateType(node->var_data.id);
                const char* rightType = inferAndValidateType(node->var_data.value);

                if (leftType == NULL || rightType == NULL) break; 
               
                type = promoteType(leftType, rightType);

                // Just to check if it is not compatible at all
                if (!type) {
                    char errorMsg[256]; 
                    snprintf(errorMsg, sizeof(errorMsg), "Type mismatch in assignment: cannot assign (%s) to (%s)", rightType, leftType);
                    addError(errorMsg, node->line_no, node->char_no);
                }

                node->inferedType = leftType; // required type
                break;
            }

        case NODE_ASSGN:
            {
                const char* leftType = inferAndValidateType(node->var_data.id);
                const char* rightType = inferAndValidateType(node->var_data.value);

                
                if (leftType == NULL || rightType == NULL) break;  

                // Apply promotion
                type = promoteType(leftType, rightType);  
                if(!type){
                    char errorMsg[256]; 
                    snprintf(errorMsg, sizeof(errorMsg), "Type mismatch in assignment: cannot assign (%s) to (%s)", rightType, leftType);
                    addError(errorMsg, node->line_no, node->char_no);
                    break;
                }

                node->inferedType = leftType;
                break;
            }
        
        case NODE_EXPR_BINARY: {
            // printf("Getting type of bin expr\n");
            const char* leftType = inferAndValidateType(node->expr_data.left);
            const char* rightType = inferAndValidateType(node->expr_data.right);
            const char* op = node->expr_data.op;

            if (leftType == NULL || rightType == NULL) break; 
            
            // Apply promotion
            type = promoteType(leftType, rightType);

             switch (getOpType(op)){
                case OP_COMP:
                    if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0){
                        // if one of the types is str and other is not
                        if (!type) {
                            char errorMsg[256];
                            snprintf(errorMsg, sizeof(errorMsg), 
                                    "Type mismatch: cannot apply operator (%s) to (%s) and (%s)", node->expr_data.op,
                                    leftType, rightType);
                            addError(errorMsg, node->line_no, node->char_no);
                        }else{
                            type = TYPE_INT;
                            node->inferedType = type;
                        }
                        
                    }
                    // Cannot apply any other comp op for strs.
                    else if(strcmp(type, TYPE_STRING) == 0){
                        char errorMsg[256];
                        snprintf(errorMsg, sizeof(errorMsg), 
                                "Type mismatch: cannot apply operator (%s) to (%s) and (%s)", node->expr_data.op,
                                leftType, rightType);
                        addError(errorMsg, node->line_no, node->char_no); 
                    }
                    break;

                case OP_ARITHMETIC:
                    if (!type || strcmp(type, TYPE_STRING) == 0) {
                        char errorMsg[256];
                        snprintf(errorMsg, sizeof(errorMsg), 
                                "Type mismatch: cannot apply operator (%s) to (%s) and (%s)", node->expr_data.op,
                                leftType, rightType);
                        addError(errorMsg, node->line_no, node->char_no);
                    }
                    node->inferedType = type; 
                    break;

                case OP_LOGICAL:
                    type = TYPE_INT;
                    node->inferedType = type;
                    break;
                
                default:
                    node->inferedType = type;
                    break;
            }

            break;
        }

        case NODE_EXPR_UNARY: {
            type = inferAndValidateType(node->expr_data.left);
            const char* op = node->expr_data.op;

            if(!type) break;

            // Cannot apply ++, -- to any other node other than ID_REF of non-str type
            switch(getOpType(op)){
                case OP_INC_DEC:
                {   
                    NodeType nType = node->expr_data.left->expr_data.left->type; // UnaryExpr --> TermExpr
                    if( nType != NODE_ID_REF || strcmp(type, TYPE_STRING) == 0){
                        char errorMsg[256];
                        snprintf(errorMsg, sizeof(errorMsg), "Type mismatch: cannot apply operator (%s) to (%s)", node->expr_data.op, type);
                        addError(errorMsg, node->line_no, node->char_no);
                    }else{
                        node->inferedType = TYPE_INT;
                    }
                    break;
                }

                case OP_ARITHMETIC:
                {   
                    if(strcmp(op, "-") == 0 && strcmp(type, TYPE_STRING) == 0){
                        char errorMsg[256];
                        snprintf(errorMsg, sizeof(errorMsg), "Type mismatch: cannot apply operator (%s) to (%s)", node->expr_data.op, type);
                        addError(errorMsg, node->line_no, node->char_no); 
                    }else{
                        node->inferedType = TYPE_INT;
                    }
                    break;
                }
                case OP_COMP:
                    node->inferedType = TYPE_INT;
                    break;
                case OP_LOGICAL:
                    node->inferedType = TYPE_INT;
                    break;      
                default:
                    node->inferedType = type;
                    break;
            }
            
            break;
        }

        default:
            printf("Unknown node type at line %d\n", node->line_no);
            type = NULL;
            break;
    }



    return type;
}


// Define the callback function for type validation
int validateTypesCallback(ASTNode* node, void* context) {
    if (!node) return 1;

    switch (node->type) {
        case NODE_VAR:{
            // printf("Validating variable\n");

            // Check if value exists
            if(!node->var_data.value) return 0;

            const char* type = inferAndValidateType(node);
            return 0; // STOP TRAVERSAL
        }

        case NODE_ASSGN: {
            // printf("Validating assignment\n");
            const char* type = inferAndValidateType(node);
            return 0;
        }

        case NODE_EXPR_BINARY: {
            // printf("Validating bin expr\n");
            const char* type = inferAndValidateType(node);
            return 0;
        }

        case NODE_EXPR_UNARY: {
            const char* type = inferAndValidateType(node);
            return 0;
        }

        default:
            break;
    }

    return 1; // Continue traversing
}

// The main validateTypes function that calls traverseAST with the callback
void validateTypes(ASTNode* root) {
    traverseAST(root, validateTypesCallback, NULL);
}
