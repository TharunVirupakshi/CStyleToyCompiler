#include "icg.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int tempVarCounter = 0;
int labelCounter = 0;

bool isDebug = false;
void setICGDebugger(){
    isDebug = true;
}

TACList* codeList;

// Runner
void startICG(ASTNode* root){
   printf("Generating ICG...\n");
   codeList = createTACList();
   TAC* head = generateCode(root);
   if(isDebug) printf("ICG Done\n");
}

// Generate a new temporary variable name
char* newTempVar() {
    char* temp = malloc(8);
    snprintf(temp, 8, "t%d", tempVarCounter++);
    return temp;
}

// Generate a new label name
char* newLabel() {
    char* label = malloc(8);
    snprintf(label, 8, "L%d", labelCounter++);
    return label;
}


// Create a new TAC instruction
TAC* createTAC(TACOp op, char* result, Operand* operand1, Operand* operand2) {
    TAC* instr = (TAC*)malloc(sizeof(TAC));
    if (!instr) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    } 
    if(isDebug) printf("Creating TAC\n");
    instr->op = op;
    instr->result = result ? strdup(result) : NULL;
    instr->operand1 = operand1;
    instr->operand2 = operand2;
    instr->next = NULL;
    if(isDebug) printf("Created TAC\n");
    return instr;   
}

Operand* makeOperand(ValueType type, void* val){
    if(isDebug) printf("Making operand\n");

    if (val == NULL) {
        fprintf(stderr, "Error: val is NULL\n");
        return NULL;
    }

    Operand* opr = (Operand*)malloc(sizeof(Operand));
    if (!opr) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    opr->type = type;
    switch (type) {
        case INT_VAL:{
            // printf("Int val\n");
            int int_val = *((int*)val);
            opr->int_val = int_val;
            if (isDebug) printf("Set opr val: %d\n", opr->int_val);
            break;
        }
        case CHAR_VAL:
            opr->char_val = *((char*)val);
            break;
        case STR_VAL:
            opr->str_val = strdup((char*)val);
            if (!opr->str_val) {
                fprintf(stderr, "Memory allocation for string failed\n");
                free(opr);
                exit(1);
            }
            break;
        case ID_REF:
            opr->id_ref = strdup((char*)val);
            if (!opr->id_ref) {
                fprintf(stderr, "Memory allocation for string failed\n");
                free(opr);
                exit(1);
            }
            break;
        default:
            fprintf(stderr, "Unknown operand type\n");
            free(opr);
            exit(1);
    }

    return opr;
}

char* getFormattedValueFromOperand(Operand* opr) {
    char* val = NULL;
    if(!opr) return val;

    if (opr->type == INT_VAL) {
        if(isDebug) printf("Formatting int %d to string\n", opr->int_val);
        val = (char*)malloc(20);  // Allocate enough space for an integer
        if (val) sprintf(val, "%d", opr->int_val);
    } else if (opr->type == CHAR_VAL) {
        if(isDebug) printf("Formatting char %c to string\n", opr->char_val);
        val = (char*)malloc(4);  // Allocate enough space for a char (e.g., "'a'")
        if (val) sprintf(val, "'%c'", opr->char_val);
    } else if (opr->type == STR_VAL) {
        if(isDebug) printf("Formatting string to string\n");
        size_t len = strlen(opr->str_val) + 3;  // Account for quotes and null terminator
        val = (char*)malloc(len);
        if (val) sprintf(val, "\"%s\"", opr->str_val);
    } else if (opr->type == ID_REF) {
        if(isDebug) printf("Formatting id_ref to string\n"); 
        size_t len = strlen(opr->id_ref) + 1;  // For ID reference without quotes
        val = (char*)malloc(len);
        if (val) sprintf(val, "%s", opr->id_ref);
    }

    if (!val) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    return val;
}


TACList* createTACList(){
    TACList* list = (TACList*)malloc(sizeof(TACList));
    list->head = NULL;
    list->tail = NULL;
    return list;
}


void appendTAC(TACList* list, TAC* newTAC) {
    if (!list->head) {
        // First node in the list
        // if(isDebug) printf("Created head tac\n");
        list->head = newTAC;
    } else {
        // Add to the end and update tail
        list->tail->next = newTAC;
    }
    list->tail = newTAC;
    if(isDebug) printf("Append TAC success\n");
}






// Generate code for a binary expression
TAC* generateCodeForBinaryExpr(ASTNode* node) {
    if(isDebug) printf("GenCode for BIN EXPR\n"); 
    
    if (node->type != NODE_EXPR_BINARY) return NULL;

    

    Operand* opr1;
    Operand* opr2;


    if(node->expr_data.left->type == NODE_EXPR_TERM){
        attachValueOfExprTerm(node->expr_data.left, &opr1);
    }else{
        TAC* leftCode = generateCode(node->expr_data.left);
        opr1 = makeOperand(ID_REF, leftCode->result); 
    }

    if(node->expr_data.right->type == NODE_EXPR_TERM){
        attachValueOfExprTerm(node->expr_data.right, &opr2);
    }else{
        TAC* rightCode = generateCode(node->expr_data.right);
        opr2 = makeOperand(ID_REF, rightCode->result); 
    }


    if(!(node->expr_data.op)){
        fprintf(stderr, "Operator is NULL\n");
        exit(1); 
    }

    const char* op = node->expr_data.op;
    TACOp tac_op;

    switch (getOpType(op)) {
        case OP_ARITHMETIC:
            if(strcmp(op, "+") == 0)        tac_op = TAC_ADD;
            else if(strcmp(op, "-") == 0)   tac_op = TAC_SUB;
            else if(strcmp(op, "*") == 0)   tac_op = TAC_MUL;
            else if(strcmp(op, "/") == 0)   tac_op = TAC_DIV; 
            break;
        

        case OP_LOGICAL:
            if (strcmp(op, "&&") == 0)      tac_op = TAC_AND; 
            else if (strcmp(op, "||") == 0) tac_op = TAC_OR; 
            break;

        case OP_COMP:
            if (strcmp(op, "==") == 0)      tac_op = TAC_EQ; 
            else if (strcmp(op, "!=") == 0) tac_op = TAC_NEQ; 
            else if (strcmp(op, "<") == 0)  tac_op = TAC_LT; 
            else if (strcmp(op, "<=") == 0) tac_op = TAC_LEQ; 
            else if (strcmp(op, ">") == 0)  tac_op = TAC_GT; 
            else if (strcmp(op, ">=") == 0) tac_op = TAC_GEQ; 
            break;

        default:
        fprintf(stderr, "Unsupported binary operator\n");
        exit(1);
    }

    TAC* newTac = createTAC(tac_op, newTempVar(), opr1, opr2);
    appendTAC(codeList, newTac); 

    return newTac;
}

void attachValueOfExprTerm(ASTNode* node, Operand** opr){
    if(isDebug) printf("Extracting val from EXPR_TERM\n");
    if(node->type != NODE_EXPR_TERM) return;

    ASTNode* valNode = node->expr_data.left;
        if(valNode->type == NODE_INT_LITERAL){
            if(isDebug) printf("Extracting INT_LITERAL\n"); 
            int temp = valNode->literal_data.value.int_value; 
            *opr = makeOperand(INT_VAL, &temp);
            if(isDebug) printf("Attached %d\n", (*opr)->int_val);
        }else if(valNode->type == NODE_CHAR_LITERAL){
            if(isDebug) printf("Extracting CHAR_LITERAL\n"); 
            char temp = valNode->literal_data.value.char_value;
            *opr = makeOperand(CHAR_VAL, &temp); 
        }else if(valNode->type == NODE_STR_LITERAL){
            if(isDebug) printf("Extracting STR_LITERAL\n"); 
            *opr = makeOperand(STR_VAL, (char*)valNode->literal_data.value.str_value);
        }else if(valNode->type == NODE_ID_REF){
            if(isDebug) printf("Extracting ID_REF\n");
            *opr = makeOperand(ID_REF, (char*)valNode->id_ref_data.name);
        }
}

TAC* genCodeForUnaryExpr(ASTNode* node){
    if(isDebug) printf("GenCode for EXPR_UNARY\n");
    if(node->type != NODE_EXPR_UNARY) return NULL;

    if(!(node->expr_data.op)){
        fprintf(stderr, "Operator is NULL\n");
        exit(1); 
    }

    TAC* rhsCode;
    Operand* opr1;
    
    if(node->expr_data.left->type == NODE_EXPR_TERM){
        attachValueOfExprTerm(node->expr_data.left, &opr1);
    }else{
        rhsCode = generateCode(node->expr_data.left);
        opr1 = makeOperand(ID_REF,rhsCode->result);
    }  

    const char* node_op = node->expr_data.op;
    TACOp op;
    if(strcmp(node_op, "-") == 0)                op = TAC_NEG;
    else if(strcmp(node_op, "!") == 0)           op = TAC_NOT;
    else if(strcmp(node_op, "POST_INC") == 0){
        // Assign the original value
        op = TAC_ASSIGN;
        char* result = newTempVar();
        TAC* newTac = createTAC(op, result, opr1, NULL);
        appendTAC(codeList, newTac); 

        // Increment later
        int temp_val = 1;
        Operand* opr2 = makeOperand(INT_VAL, &temp_val);
        TAC* postInc = createTAC(TAC_ADD, opr1->id_ref, opr1, opr2);

        appendTAC(codeList, postInc);
        return newTac;
    }   
    else if(strcmp(node_op, "POST_DEC") == 0){
        // Assign the original value
        op = TAC_ASSIGN;
        char* result = newTempVar();
        TAC* newTac = createTAC(op, result, opr1, NULL);
        appendTAC(codeList, newTac);  

        // Decrement later
        int temp_val = 1;
        Operand* opr2 = makeOperand(INT_VAL, &temp_val);
        TAC* postInc = createTAC(TAC_SUB, opr1->id_ref, opr1, opr2);
        appendTAC(codeList, postInc);
        return newTac;
    }    
    else if(strcmp(node_op, "PRE_INC") == 0){
        // Increment First
        int temp_val = 1;
        Operand* opr2 = makeOperand(INT_VAL, &temp_val);
        TAC* postInc = createTAC(TAC_ADD, opr1->id_ref, opr1, opr2);
        appendTAC(codeList, postInc);

        // Assign the incremented value
        op = TAC_ASSIGN;
        char* result = newTempVar();
        TAC* newTac = createTAC(op, result, opr1, NULL);
        appendTAC(codeList, newTac);  
        
        return newTac;        
    }
    else if(strcmp(node_op, "PRE_DEC") == 0){
        // Decrement First
        int temp_val = 1;
        Operand* opr2 = makeOperand(INT_VAL, &temp_val);
        TAC* postInc = createTAC(TAC_SUB, opr1->id_ref, opr1, opr2);
        appendTAC(codeList, postInc);

        // Assign the decremented value
        op = TAC_ASSIGN;
        char* result = newTempVar();
        TAC* newTac = createTAC(op, result, opr1, NULL);
        appendTAC(codeList, newTac);  
        
        return newTac; 
    }
    else{
        fprintf(stderr, "Unsupported operator\n");
        exit(1);  
    }    

    char* result = newTempVar();
    TAC* newTac = createTAC(op, result, opr1, NULL);

    appendTAC(codeList, newTac);

    return newTac;
}

// Generate code for an assignment statement
TAC* generateCodeForAssignment(ASTNode* node) {
    if(isDebug) printf("GenCode for NODE_ASSGN\n");
    if (node->type != NODE_ASSGN) return NULL;
    
    Operand* opr1;
    char* result = strdup(node->assgn_data.left->id_ref_data.name); 

    if(node->assgn_data.right->type == NODE_EXPR_TERM){
       attachValueOfExprTerm(node->assgn_data.right, &opr1); 
    }else{
       TAC* rhsCode = generateCode(node->assgn_data.right);
       opr1 = makeOperand(ID_REF, rhsCode->result); 
    }

    TAC* code = createTAC(TAC_ASSIGN, result, opr1, NULL);
    appendTAC(codeList, code);

    return code;
}

 

TAC* genCodeForVar(ASTNode* node){
    if(isDebug) printf("GenCode for VAR\n");
    if(node->type != NODE_VAR) return NULL;


    Operand* opr1;

    char* result = strdup(node->var_data.id->id_data.sym->name);

    if(node->var_data.value == NULL){
        opr1 = NULL;
    }else{
        if(node->var_data.value->type == NODE_EXPR_TERM){
            attachValueOfExprTerm(node->var_data.value, &opr1);
        }else{
            TAC* rhsCode = generateCode(node->var_data.value);
            opr1 = makeOperand(ID_REF, rhsCode->result);
        }    

    }
    TAC* code = createTAC(TAC_ASSIGN, result, opr1, NULL);
    
    appendTAC(codeList, code);

    return code;
}

TAC* genCodeForVarList(ASTNode* node){
    if(isDebug) printf("GenCode for VAR_LIST\n");
    if(!node) return NULL;
    if(node->type != NODE_VAR_LIST) return NULL;

    TAC* code_var_list = genCodeForVarList(node->var_list_data.var_list);
    TAC* code_var = genCodeForVar(node->var_list_data.var);
    return code_var;
}


TAC* generateCodeForDecl(ASTNode* node){
    if(isDebug) printf("GenCode for DECL\n");
    if(node->type != NODE_DECL) return NULL;

    TAC* code_var_list = genCodeForVarList(node->decl_data.var_list);
    return code_var_list;
}

// Main function to generate TAC code for an AST node
TAC* generateCode(ASTNode* node) {
    if (!node) return NULL;

    switch (node->type) {
        case NODE_PROGRAM:{
            return generateCode(node->program_data.stmt_list);
        }
        case NODE_STMT_LIST:{
            generateCode(node->stmt_list_data.stmt_list);
            return generateCode(node->stmt_list_data.stmt);
        }
        case NODE_STMT:{
            return generateCode(node->stmt_data.stmt);
        }
        case NODE_DECL:{
            return generateCodeForDecl(node);
        }
        case NODE_EXPR_BINARY:
            return generateCodeForBinaryExpr(node);

        case NODE_EXPR_UNARY:
            return genCodeForUnaryExpr(node);
        case NODE_EXPR_TERM:
            return NULL;
        case NODE_ASSGN:
            return generateCodeForAssignment(node);
            // return generateCodeForAssignment(node);
        default:
            fprintf(stderr, "Unsupported AST node type\n");
            exit(1);
    }
}

// Function to print the TAC instruction based on its operation type
void printTACInstruction(TAC* instr) {
    if (!instr) return;

    char* opr1 = getFormattedValueFromOperand(instr->operand1);
    char* opr2 = getFormattedValueFromOperand(instr->operand2);

    switch (instr->op) {
        case TAC_ASSIGN:
            printf("%s = %s\n", instr->result, opr1);
            break;
        case TAC_ADD:
            printf("%s = %s + %s\n", instr->result, opr1, opr2);
            break;
        case TAC_SUB:
            printf("%s = %s - %s\n", instr->result, opr1, opr2);
            break;
        case TAC_MUL:
            printf("%s = %s * %s\n", instr->result, opr1, opr2);
            break;
        case TAC_DIV:
            printf("%s = %s / %s\n", instr->result, opr1, opr2);
            break;
        case TAC_AND:
            printf("%s = %s AND %s\n", instr->result, opr1, opr2);
            break;
        case TAC_OR:
            printf("%s = %s OR %s\n", instr->result, opr1, opr2);
            break;
        case TAC_NEG:
            printf("%s = -%s\n", instr->result, opr1);
            break;        
        case TAC_NOT:
            printf("%s = !%s\n", instr->result, opr1);
            break; 
        default:
            fprintf(stderr, "Unknown TAC operation\n");
            break;
    }
}

// Function to print the entire TAC linked list
void printTAC() {
    if(!codeList){
        printf("No CodeList\n");
        return;
    }
    if(isDebug) printf("Printing TAC\n");
    TAC* current = codeList->head;
    while (current) {
        printTACInstruction(current);
        current = current->next;
    }
}