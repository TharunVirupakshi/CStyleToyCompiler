#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define TYPE_VOID "void"
#define TYPE_INT "int"
#define TYPE_CHAR "char"
#define TYPE_STRING "string"
#define TYPE_UNKNOWN "unknown"

bool isDebugOn = false;
void setSemanticDebugger(){
    isDebugOn = true;
} 


OpType getOpType(const char* op) {

    if(!op) return OP_UNKNOWN;

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


const char* inferAndValidateType(ASTNode* node); 
// const char* getNodeTypeName(ASTNode* node);

// Main function
SemanticStatus performSemanticAnalysis(ASTNode* root, SymbolTable* globalTable, BrkCntStmtsList* list) {
    if (!root) return SEMANTIC_ERROR;

    checkDuplicates(globalTable);
    validateSymbolUsage(root);
    validateLoops(root, list);

    if(errorCount > 0){
        printErrors();
        errorCount = 0;
        return SEMANTIC_ERROR;
    }

    validateTypes(root);

    if(errorCount > 0){
        printErrors();
        errorCount = 0;
        return SEMANTIC_ERROR;
    }

    validateFunctionReturnTypes(root);

    if(errorCount > 0){
        printErrors();
        errorCount = 0;
        return SEMANTIC_ERROR;
    }

    return SEMANTIC_SUCCESS;
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
        if(errorList[i].message){
            printf("Error: %s at (line %d, char %d)\n", errorList[i].message, errorList[i].line, errorList[i].char_no);
            free(errorList[i].message); // Free each error message after printing
        }
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


int validateSymbolUsageCallback(ASTNode* root, void* cxt){
    if (!root) return 0;

    // If the node is an identifier (e.g., variable reference)
    if (root->type == NODE_ID_REF) {
        const char* varName = root->id_ref_data.name;  // Assuming the ID node stores the name in `value.id`
        if(isDebugOn) printf("Validating symbol %s\n", varName);
        // Look up the symbol in the current or any parent scope
        SymbolTable* currentScope = root->id_ref_data.scope;
        symbol* foundSymbol = lookupSymbol(currentScope, varName);

        // If the symbol is not found, it's an undeclared variable
        if (!foundSymbol) {
            char errorMsg[256];
            snprintf(errorMsg, sizeof(errorMsg), "Undeclared variable '%s'", varName);
            addError(errorMsg, root->line_no, root->char_no);  // Adding error with line and char info
            root->id_ref_data.ref = NULL; 
            assert(root->id_ref_data.ref == NULL);
            return 0;
        }else{
            // Bind the id_ref node to sym
            if(isDebugOn) printf("Found ref for %s in scope %s\n", varName, foundSymbol->scope->scopeName);
            assert(foundSymbol != NULL);
            root->id_ref_data.ref = foundSymbol;
        }
    }

    return 1;
}

void validateSymbolUsage(ASTNode* root){

    traverseAST(root, validateSymbolUsageCallback, NULL);
    // Traverse the children
    // Traverse each type of node according to the specific structure
    // switch (root->type) {
    //     case NODE_PROGRAM:
    //         validateSymbolUsage(root->program_data.stmt_list);
    //         break;

    //     case NODE_RETURN:
    //         validateSymbolUsage(root->return_data.return_value);
    //         break;

    //     case NODE_STMT_LIST:
    //         validateSymbolUsage(root->stmt_list_data.stmt_list);
    //         validateSymbolUsage(root->stmt_list_data.stmt);
    //         break;

    //     case NODE_STMT:
    //         validateSymbolUsage(root->stmt_data.stmt);
    //         break;

    //     case NODE_BLOCK_STMT:
    //         validateSymbolUsage(root->block_stmt_data.stmt_list);
    //         break;

    //     case NODE_DECL:
    //         // validateSymbolUsage(root->decl_data.type_spec);
    //         validateSymbolUsage(root->decl_data.var_list);
    //         break;

    //     case NODE_VAR_LIST:
    //         validateSymbolUsage(root->var_list_data.var_list);
    //         validateSymbolUsage(root->var_list_data.var);
    //         break;

    //     case NODE_VAR:
    //         validateSymbolUsage(root->var_data.id);
    //         validateSymbolUsage(root->var_data.value);
    //         break;

    //     case NODE_ASSGN:
    //         validateSymbolUsage(root->assgn_data.left);
    //         validateSymbolUsage(root->assgn_data.right);
    //         break;

    //     case NODE_EXPR_BINARY:
    //         validateSymbolUsage(root->expr_data.left);
    //         validateSymbolUsage(root->expr_data.right);
    //         break;

    //     case NODE_EXPR_UNARY:
    //     case NODE_EXPR_TERM:
    //         validateSymbolUsage(root->expr_data.left);
    //         break;

    //     case NODE_IF:
    //         validateSymbolUsage(root->if_else_data.condition);
    //         validateSymbolUsage(root->if_else_data.if_branch);
    //         break;

    //     case NODE_IF_ELSE:
    //         validateSymbolUsage(root->if_else_data.condition);
    //         validateSymbolUsage(root->if_else_data.if_branch);
    //         validateSymbolUsage(root->if_else_data.else_branch);
    //         break;

    //     case NODE_IF_COND:
    //         validateSymbolUsage(root->if_cond_data.cond);
    //         break;
        
    //     case NODE_IF_BRANCH:
    //     case NODE_ELSE_BRANCH:
    //         validateSymbolUsage(root->if_else_branch.branch);
    //         break;
        

    //     case NODE_WHILE:
    //         validateSymbolUsage(root->while_data.condition);
    //         validateSymbolUsage(root->while_data.while_body);
    //         break;

    //     case NODE_WHILE_COND:   
    //         validateSymbolUsage(root->while_cond_data.cond);
    //         break;

    //     case NODE_WHILE_BODY:   
    //         validateSymbolUsage(root->while_body_data.body);
    //         break;

    //     case NODE_FOR:
    //         validateSymbolUsage(root->for_data.init);
    //         validateSymbolUsage(root->for_data.condition);
    //         validateSymbolUsage(root->for_data.updation);
    //         validateSymbolUsage(root->for_data.body);
    //         break;

    //     case NODE_FOR_INIT:
    //         validateSymbolUsage(root->for_init_data.init);
    //         break;

    //     case NODE_FOR_COND: 
    //         validateSymbolUsage(root->for_cond_data.cond); 
    //         break;

    //     case NODE_FOR_UPDATION: 
    //         validateSymbolUsage(root->for_updation_data.updation); 
    //         break;

    //     case NODE_FOR_BODY: 
    //         validateSymbolUsage(root->for_body_data.body); 
    //         break; 

    //     case NODE_EXPR_COMMA_LIST:
    //         validateSymbolUsage(root->expr_comma_list_data.expr_comma_list);
    //         validateSymbolUsage(root->expr_comma_list_data.expr_comma_list_item);
    //         break;
        

    //     case NODE_FUNC_DECL:
    //         validateSymbolUsage(root->func_decl_data.params);
    //         validateSymbolUsage(root->func_decl_data.body);
    //         break;
        
    //     case NODE_FUNC_BODY:
    //         validateSymbolUsage(root->func_body_data.body);
    //         break;

    //     case NODE_FUNC_CALL:
    //         validateSymbolUsage(root->func_call_data.id);
    //         validateSymbolUsage(root->func_call_data.arg_list);
    //         break;

    //     // case NODE_PARAM_LIST:
    //     //     validateSymbolUsage(root->param_list_data.param);
    //     //     validateSymbolUsage(root->param_list_data.param_list);
    //     //     break;

    //     case NODE_ARG_LIST:
    //         validateSymbolUsage(root->arg_list_data.arg_list);
    //         validateSymbolUsage(root->arg_list_data.arg);
    //         break;

    //     case NODE_ARG:
    //         validateSymbolUsage(root->arg_data.arg);
    //         break;

    //     default:
    //         break;
    // }
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


void validateFunctionCallArgs(ASTNode* func_call_node) {
    // Get function call identifier and its symbol
    if(!func_call_node){
        if(isDebugOn) printf("func call node is NULL\n");
        return;
    }

    if(isDebugOn) printf("Validating func call args for '%s()'\n", func_call_node->func_call_data.id->id_ref_data.name);

    symbol* func_symbol = func_call_node->func_call_data.id->id_ref_data.ref;
    if(isDebugOn){
         if(!func_symbol) printf("Func sym not found!\n");
    }
    if (!func_symbol || !func_symbol->is_function) {
        addError("Called identifier is not a function", func_call_node->line_no, func_call_node->char_no);
        return;
    }

    // Retrieve the function declaration node
    ASTNode* func_decl_node = func_symbol->func_node;
    int expected_count = func_decl_node->func_decl_data.param_count;

    // Argument count validation
    if (func_call_node->func_call_data.arg_count != expected_count) {
        char errorMsg[256];
        snprintf(errorMsg, sizeof(errorMsg),
                 "Argument count mismatch for function '%s': expected %d, got %d",
                 func_symbol->name, expected_count, func_call_node->func_call_data.arg_count);
        addError(errorMsg, func_call_node->line_no, func_call_node->char_no);
        return;
    }

    // Initialize pointers to traverse arguments and parameters
    ASTNode* arg_node = func_call_node->func_call_data.arg_list;
    ASTNode* param_node = func_decl_node->func_decl_data.params;
    int arg_index = expected_count-1;


    while (arg_node && param_node) {

        if (arg_node->arg_list_data.arg == NULL || arg_node->arg_list_data.arg->arg_data.arg == NULL) {
            addError("Invalid argument node structure", arg_node->line_no, arg_node->char_no);
            return;
        }

        // Get expected parameter type
        const char* expected_type = param_node->param_list_data.param->param_data.type_spec->type_data.type;

        // Infer argument type
        const char* arg_type = inferAndValidateType(arg_node->arg_list_data.arg->arg_data.arg);

        if(isDebugOn){
            if(!arg_type) printf("Type of Arg %d is NULL\n", arg_index);
            if(!expected_type) printf("Type of Param %d is NULL\n", arg_index);
        }

        if(expected_type && arg_type){
            // Type mismatch check
            if (strcmp(arg_type, expected_type) != 0) {
                char errorMsg[256];
                snprintf(errorMsg, sizeof(errorMsg),
                        "Type mismatch in argument %d for function '%s': expected (%s), got (%s)",
                        arg_index + 1, func_symbol->name, expected_type, arg_type);
                addError(errorMsg, arg_node->arg_list_data.arg->line_no, arg_node->arg_list_data.arg->char_no);
            }
        }

     

        // Move to the next argument and parameter
        arg_node = arg_node->arg_list_data.arg_list;
        param_node = param_node->param_list_data.param_list;
        arg_index--;
    }

    return;  // Arguments validated successfully
}


const char* inferAndValidateType(ASTNode* node) {

    const char* type = NULL;

    if (!node) return NULL;
    
    switch (node->type) {
        case NODE_ID:
            if(isDebugOn) printf("Getting type of ID (%s)\n", node->id_data.sym->name);

            type = node->id_data.sym->type;
            if(!type){
                char errorMsg[256]; 
                snprintf(errorMsg, sizeof(errorMsg),"Type of '%s' is NULL", node->id_data.sym->name);
                addError(errorMsg, node->line_no, node->char_no);
            } 
            node->inferedType = type;
            break;

        case NODE_ID_REF:
            if(isDebugOn) printf("Getting type of ID ref (%s)\n", node->id_ref_data.name);

            if (node->id_ref_data.ref != NULL){
                if(isDebugOn) printf("ref type is not NULL\n");
                type = node->id_ref_data.ref->type ? node->id_ref_data.ref->type : NULL;
            }else{
                if(isDebugOn) printf("ref type is NULL\n");
                type = NULL;
            }

            if(!type){
                char errorMsg[256]; 
                snprintf(errorMsg, sizeof(errorMsg),"Type of '%s' is NULL", node->id_ref_data.name);
                addError(errorMsg, node->line_no, node->char_no);
                return NULL;
            } 
            node->inferedType = type;            
            break;

        case NODE_INT_LITERAL:
            if(isDebugOn) printf("Getting type of int (%d)\n", node->literal_data.value.int_value);
            type = TYPE_INT;
            node->inferedType = type;
            break;

        case NODE_CHAR_LITERAL:
            if(isDebugOn) printf("Getting type of char (%c)\n",  node->literal_data.value.char_value);
            type = TYPE_CHAR;
            node->inferedType = type;
            break;

        case NODE_STR_LITERAL:
            if(isDebugOn) printf("Getting type of str\n");
            type = TYPE_STRING;
            node->inferedType = type;
            break;

        case NODE_EXPR_TERM:
            if(isDebugOn) printf("Getting type of expr term\n");
            type = inferAndValidateType(node->expr_data.left);
            node->inferedType = type;
            break;

        case NODE_FUNC_CALL:
            if(isDebugOn) printf("Getting type of func call\n");
            type = inferAndValidateType(node->func_call_data.id);
            node->inferedType = type;

            // validate the args
            validateFunctionCallArgs(node);
            break;

        case NODE_RETURN:
            if(isDebugOn) printf("Getting type of return stmt\n");
            type = inferAndValidateType(node->return_data.return_value);
            node->inferedType = type ? type : TYPE_VOID;
            type = TYPE_VOID;
            break;

        case NODE_VAR:
            {   
                if(isDebugOn) printf("Getting type of var node\n");
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
                if(isDebugOn) printf("Getting type of assgn node\n");
                const char* leftType = inferAndValidateType(node->assgn_data.left);
                const char* rightType = inferAndValidateType(node->assgn_data.right);

                
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
            if(isDebugOn) printf("Getting type of bin expr op(%s)\n", node->expr_data.op);
            const char* leftType = inferAndValidateType(node->expr_data.left);
            const char* rightType = inferAndValidateType(node->expr_data.right);
            const char* op = node->expr_data.op;

            if (leftType == NULL || rightType == NULL || op == NULL) break; 
            
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
                    else if(!type || strcmp(type, TYPE_STRING) == 0){
                        char errorMsg[256];
                        snprintf(errorMsg, sizeof(errorMsg), 
                                "Type mismatch: cannot apply operator (%s) to (%s) and (%s)", node->expr_data.op,
                                leftType, rightType);
                        addError(errorMsg, node->line_no, node->char_no); 
                        type = NULL;
                    }
                    else{
                            type = TYPE_INT;
                            node->inferedType = type;
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
            if(isDebugOn) printf("Getting type of unary expr op(%s)\n", node->expr_data.op);
            type = inferAndValidateType(node->expr_data.left);
            const char* op = node->expr_data.op;

            if(!type || !op) break;

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
                        type = TYPE_INT;
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
                        type = TYPE_INT;
                        node->inferedType = TYPE_INT;
                    }
                    break;
                }
                case OP_COMP:
                    type = TYPE_INT;
                    node->inferedType = TYPE_INT;
                    break;
                case OP_LOGICAL:
                    type = TYPE_INT;
                    node->inferedType = TYPE_INT;
                    break;      
                default:
                    type = NULL;
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

        case NODE_EXPR_TERM: {
            const char* type = inferAndValidateType(node);
            return 0;
        }

        case NODE_RETURN: {
            const char* type = inferAndValidateType(node);
            return 0;
        }

        case NODE_FUNC_CALL: {
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







int validateBreakContinueStmtsCallback(ASTNode* node, void* ctx){
    if(node->type == NODE_WHILE || node->type == NODE_FOR) return 0;

    if(node->type == NODE_BREAK_STMT || node->type == NODE_CONTINUE_STMT){
        ASTNode* loop_node = (ASTNode*)(ctx);
        node->break_continue_stmt_data.associated_loop_node = loop_node;

        return 0;
    } 

    return 1;
}

int validateLoopsCallback(ASTNode* node, void* context){
    if(node->type == NODE_WHILE){
        traverseAST(node->while_data.while_body->while_body_data.body, validateBreakContinueStmtsCallback, node);
    }else if(node->type == NODE_FOR){
        traverseAST(node->for_data.body->for_body_data.body, validateBreakContinueStmtsCallback, node);
    }

    return 1;
}

void validateLoops(ASTNode* node, BrkCntStmtsList* list){
    traverseAST(node, validateLoopsCallback, NULL);

    // Check for invalid/unassigned break/continue stmts
    BrkCntStmtsList* temp = list;
    while(temp){
        if(temp->node->type == NODE_BREAK_STMT || temp->node->type == NODE_CONTINUE_STMT){
            if(temp->node->break_continue_stmt_data.associated_loop_node == NULL){
                char errorMsg[256];
                char* type = temp->node->type == NODE_BREAK_STMT ? "break" : temp->node->type == NODE_CONTINUE_STMT ? "continue" : " ";
                snprintf(errorMsg, sizeof(errorMsg), "\"%s\" must be within loop body", type);
                addError(errorMsg, node->line_no, node->char_no); 
            }
        }
        temp = temp->next;
    }
}


typedef struct {
    ASTNode* func_decl_node;
    bool ret_found;
    const char* expected_type;
} RetValCtx;

int validateReturnStmtsCallback(ASTNode* node, void* ctx){

    if(node->type == NODE_FUNC_DECL) return 0; // Stop traversing this branch

    if(node->type == NODE_RETURN){
        RetValCtx* val_ctx = (RetValCtx*)(ctx);
        val_ctx->ret_found = true;
        const char* ret_type = node->inferedType; // Assumed that the type is already infered by previous phase
        const char* expected_type = val_ctx->expected_type;

        if(!ret_type || !expected_type) return 0;

        if(strcmp(ret_type, expected_type) != 0){
            char errorMsg[256];
            snprintf(errorMsg, sizeof(errorMsg),
                     "Return type mismatch: expected (%s), got (%s) for '%s()'",
                     expected_type, ret_type, val_ctx->func_decl_node->func_decl_data.id->id_data.sym->name);
            addError(errorMsg, node->line_no, node->char_no);
            
        }

        // Bind the reutrn stmt with the associated node
        node->return_data.associated_node = val_ctx->func_decl_node;

        return 0; // Stop traversing further down this branch
    }

    return 1; // Traverse other nodes
}


int validateFuncRetTypesCallback(ASTNode* node, void* context){
    // Only handle function declarations
    if (node->type != NODE_FUNC_DECL) return 1; // Skip non func_decl nodes and continue traversing

    RetValCtx ret_ctx = {
        .func_decl_node = node,
        .ret_found = false,
        .expected_type = node->func_decl_data.id->id_data.sym->type
    };

    if(!(ret_ctx.expected_type)){
        char errorMsg[256];
        snprintf(errorMsg, sizeof(errorMsg),
                "Type of '%s' is NULL",
                node->func_decl_data.id->id_data.sym->name);
        addError(errorMsg, node->line_no, node->char_no); 

        return 1;
    }

    // Traverse only the function body to find return statements
    traverseAST(node->func_decl_data.body, validateReturnStmtsCallback, &ret_ctx);

    // Non-void functions must have return stmts
    // if(!(ret_ctx.ret_found) && strcmp(ret_ctx.expected_type, TYPE_VOID) != 0 ){
    //     char errorMsg[256];
    //     snprintf(errorMsg, sizeof(errorMsg),
    //             "Missing return statement in function '%s' with (non-void) return type",
    //             node->func_decl_data.id->id_data.sym->name);
    //     addError(errorMsg, node->line_no, node->char_no); 
    // }

    return 1; // Look for other func declarations
}
void validateFunctionReturnTypes(ASTNode* root){
    traverseAST(root, validateFuncRetTypesCallback, NULL);
}

