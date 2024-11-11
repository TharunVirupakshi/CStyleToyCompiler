#include "icg.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int tempVarCounter = 0;
int labelCounter = 0;
int instructionCounter = 1;

// Function to get the index of the next instruction
int getNextInstruction() {
    return instructionCounter;
}

bool isDebug = false;
void setICGDebugger(){
    isDebug = true;
}

BoolExprInfo global_bool_info = {NULL, NULL};

TACList* codeList;

// Runner
void startICG(ASTNode* root){
    if(isDebug) printf("[DEBUG] Starting ICG generation...\n");
    codeList = createTACList();
    generateCode(root, &global_bool_info);
    if(isDebug) printf("[DEBUG] ICG generation completed.\n");
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

Label* createLabel(TAC* tac){
    Label* label = (Label*)malloc(sizeof(Label));
    label->name = newLabel();
    label->tac = tac;
    return label;
}


// Create a new TAC instruction
TAC* createTAC(TACOp op, char* result, Operand* operand1, Operand* operand2) {
    TAC* instr = (TAC*)malloc(sizeof(TAC));
    if (!instr) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    } 
    instr->op = op;
    instr->result = result ? strdup(result) : NULL;
    instr->operand1 = operand1;
    instr->operand2 = operand2;
    instr->next = NULL;
    if(isDebug){
        printf("[DEBUG] Created TAC for operation: %s, Result: %s\n", getOperatorString(op), result ? result : "NULL");
    } 
    return instr;   
}


const char* getOperatorString(TACOp op) {
    switch (op) {
        case TAC_ADD:            return "+";
        case TAC_SUB:            return "-";
        case TAC_MUL:            return "*";
        case TAC_DIV:            return "/";
        case TAC_AND:            return "AND";
        case TAC_OR:             return "OR";
        case TAC_NOT:            return "!";
        case TAC_NEG:            return "- (NEG)";
        case TAC_EQ:             return "==";
        case TAC_NEQ:            return "!=";
        case TAC_LT:             return "<";
        case TAC_GT:             return ">";
        case TAC_LEQ:            return "<=";
        case TAC_GEQ:            return ">=";
        case TAC_POST_INC:       return "POST_INC";
        case TAC_POST_DEC:       return "POST_DEC";
        case TAC_PRE_INC:        return "PRE_INC";
        case TAC_PRE_DEC:        return "PRE_DEC";
        case TAC_ASSIGN:         return "=";
        case TAC_LABEL:          return "LABEL";
        case TAC_GOTO:           return "GOTO";
        case TAC_IF_GOTO:        return "IF_GOTO";
        case TAC_IF_FALSE_GOTO:  return "IF_FALSE_GOTO";
        case TAC_CALL:           return "CALL";
        case TAC_PARAM:          return "PARAM";
        case TAC_RETURN:         return "RETURN";
        default:                 return "UNKNOWN_OPERATOR";
    }
}


// Create a new list with a single TAC node
List* makeList(TAC* tac) {
    if(isDebug) printf("[DEBUG] Creating a new true/false list for TAC node %d\n", tac->tac_id);
    List* newList = (List*)malloc(sizeof(List));
    if (!newList) {
        fprintf(stderr, "Memory allocation failed for makeList\n");
        exit(1);
    }
    newList->tac = tac;   // Set the TAC pointer to the given TAC instruction
    newList->next = NULL;
    return newList;
}

// Merge two lists of TAC instructions
List* merge(List* list1, List* list2) {
    if (!list1) return list2;
    if (!list2) return list1;
    if(isDebug) printf("[DEBUG] Merged two true/false lists\n");
    List* temp = list1;
    while (temp->next) {
        temp = temp->next;
    }
    temp->next = list2;
    return list1;
}

// Backpatch all TAC instructions in the list with the given target label name
void backpatch(List* list, int instr_no) {
    List* temp = list;
    while (temp) {
        if (temp->tac) {
            temp->tac->target_jump = instr_no;
            if(isDebug) printf("[DEBUG] Backpatched TAC %d to jump to instruction %d\n", temp->tac->tac_id, instr_no);
        }
        temp = temp->next;
    }
}

char* generateScopeSuffixedName(const char* name, int scope_id) {
    int suffix_len = snprintf(NULL, 0, "_%d", scope_id);
    char* suffixed_name = (char*)malloc(strlen(name) + suffix_len + 1);

    if (!suffixed_name) {
        fprintf(stderr, "Memory allocation failed for suffixed name\n");
        exit(1);
    }

    sprintf(suffixed_name, "%s_%d", name, scope_id);
    return suffixed_name;
}

Operand* makeOperand(ValueType type, const void* val){
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
            if (isDebug) printf("[DEBUG] Created INT operand with value: %d\n", opr->int_val);
            break;
        }
        case CHAR_VAL:
            opr->char_val = *((char*)val);
            if (isDebug) printf("[DEBUG] Created CHAR operand with value: %c\n", opr->char_val);
            break;
        case STR_VAL:
            opr->str_val = strdup((char*)val);
            if (!opr->str_val) {
                fprintf(stderr, "Memory allocation for string failed\n");
                free(opr);
                exit(1);
            }
            if (isDebug) printf("[DEBUG] Created STR operand with value: %s\n", opr->str_val);
            break;
        case ID_REF:
            opr->id_ref.name = strdup((char*)val);
            if (!opr->id_ref.name) {
                fprintf(stderr, "Memory allocation for string failed\n");
                free(opr);
                exit(1);
            }
            if (isDebug) printf("[DEBUG] Created ID_REF operand with name: %s\n", opr->id_ref.name);
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
        val = (char*)malloc(20);  // Allocate enough space for an integer
        if (val) sprintf(val, "%d", opr->int_val);
    } else if (opr->type == CHAR_VAL) {
        val = (char*)malloc(4);  // Allocate enough space for a char (e.g., "'a'")
        if (val) sprintf(val, "'%c'", opr->char_val);
    } else if (opr->type == STR_VAL) {
        size_t len = strlen(opr->str_val) + 3;  // Account for quotes and null terminator
        val = (char*)malloc(len);
        if (val) sprintf(val, "%s", opr->str_val);
    } else if (opr->type == ID_REF) {
        size_t len = strlen(opr->id_ref.name) + 1;  // For ID reference without quotes
        val = (char*)malloc(len);
        if (val) sprintf(val, "%s", opr->id_ref.name);
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
    newTAC->tac_id = instructionCounter;
    instructionCounter++;
    if (isDebug) printf("[DEBUG] Appended TAC %d\n", newTAC->tac_id);
}


void attachValueOfExprTerm(ASTNode* node, Operand** opr){
    if(isDebug) printf("Extracting val from EXPR_TERM\n");
    if(node->type != NODE_EXPR_TERM) return;

    ASTNode* valNode = node->expr_data.left;
        if(valNode->type == NODE_INT_LITERAL){
            int temp = valNode->literal_data.value.int_value; 
            *opr = makeOperand(INT_VAL, &temp);
            if(isDebug) printf("[DEBUG] Attached INT_LITERAL value: %d\n", (*opr)->int_val);
        }else if(valNode->type == NODE_CHAR_LITERAL){
            char temp = valNode->literal_data.value.char_value;
            *opr = makeOperand(CHAR_VAL, &temp);
            if (isDebug) printf("[DEBUG] Attached CHAR_LITERAL value: %c\n", (*opr)->char_val);    
        }else if(valNode->type == NODE_STR_LITERAL){
            *opr = makeOperand(STR_VAL, (char*)valNode->literal_data.value.str_value);
            if (isDebug) printf("[DEBUG] Attached STR_LITERAL value: %s\n", valNode->literal_data.value.str_value);
        }else if(valNode->type == NODE_ID_REF){
    
            char* suffixed_name = generateScopeSuffixedName(valNode->id_ref_data.name, valNode->id_ref_data.scope->table_id);
        
            *opr = makeOperand(ID_REF, suffixed_name);
            (*opr)->id_ref.sym = valNode->id_ref_data.ref;
            if (isDebug) printf("[DEBUG] Attached ID_REF value: %s\n", suffixed_name);
        }
}




// Generate code for a binary expression
TAC* generateCodeForBinaryExpr(ASTNode* node, BoolExprInfo* bool_info) {
    if(isDebug) printf("[DEBUG] GenCode for BIN EXPR\n"); 
    
    if (node->type != NODE_EXPR_BINARY) return NULL;


    Operand* opr1 = NULL;
    Operand* opr2 = NULL;

    if(!(node->expr_data.op)){
        fprintf(stderr, "Operator is NULL\n");
        exit(1); 
    }

    const char* op = node->expr_data.op;
    TACOp tac_op;

    switch (getOpType(op)) {
        case OP_COMP:
        case OP_ARITHMETIC:{  
            BoolExprInfo b_info = {NULL, NULL}; 

            if(node->expr_data.left->type == NODE_EXPR_TERM){
                attachValueOfExprTerm(node->expr_data.left, &opr1);
            }else{
                TAC* leftCode = generateCode(node->expr_data.left, &b_info);
                opr1 = makeOperand(ID_REF, leftCode->result); 
            }

            if(node->expr_data.right->type == NODE_EXPR_TERM){
                attachValueOfExprTerm(node->expr_data.right, &opr2);
            }else{
                TAC* rightCode = generateCode(node->expr_data.right, &b_info);
                opr2 = makeOperand(ID_REF, rightCode->result); 
            }

            if(strcmp(op, "+") == 0)        tac_op = TAC_ADD;
            else if(strcmp(op, "-") == 0)   tac_op = TAC_SUB;
            else if(strcmp(op, "*") == 0)   tac_op = TAC_MUL;
            else if(strcmp(op, "/") == 0)   tac_op = TAC_DIV; 
            else if (strcmp(op, "==") == 0) tac_op = TAC_EQ; 
            else if (strcmp(op, "!=") == 0) tac_op = TAC_NEQ; 
            else if (strcmp(op, "<") == 0)  tac_op = TAC_LT; 
            else if (strcmp(op, "<=") == 0) tac_op = TAC_LEQ; 
            else if (strcmp(op, ">") == 0)  tac_op = TAC_GT; 
            else if (strcmp(op, ">=") == 0) tac_op = TAC_GEQ; 
            
            
            TAC* newTac = createTAC(tac_op, newTempVar(), opr1, opr2);
            appendTAC(codeList, newTac); 

            return newTac;
        }
        case OP_LOGICAL:{
            if (strcmp(op, "&&") == 0){
                if(isDebug) printf("[DEBUG] Handling logical (&&) expression\n");
                //Process left child
                Operand* l_opr1 = NULL; 
                BoolExprInfo l_info = {NULL, NULL};
                TAC* leftSubCode = NULL;
                if(node->expr_data.left->type == NODE_EXPR_TERM){
                    attachValueOfExprTerm(node->expr_data.left, &l_opr1);
                }else{
                    leftSubCode = generateCode(node->expr_data.left, &l_info);
                    l_opr1 = makeOperand(ID_REF, leftSubCode->result); 
                    if(isDebug) printf("[DEBUG] Left sub expr result %s\n", leftSubCode->result);
                }

                // Generate TAC to store the boolean result in a temporary variable
              
                // Generate code for left
                TAC* leftIfFalseCode = createTAC(TAC_IF_FALSE_GOTO, NULL, l_opr1, NULL);
                // TAC* leftGotoCode = createTAC(TAC_GOTO, NULL, NULL, NULL);
                appendTAC(codeList, leftIfFalseCode);
                // appendTAC(codeList, leftGotoCode);

                // l_info.trueList = makeList(leftIfFalseCode);
                
                // If leftSubCode is NULL, then l_info will be NULL
                List* temp = NULL;
                if(isDebug){
                    printf("[DEBUG] Left sub expr truelist: ");
                    temp = l_info.trueList;
                    while(temp){
                        printf("%d ", temp->tac->tac_id);
                        temp = temp->next;
                    }
                    printf("\n[DEBUG] Left sub expr falselist: ");
                    temp = l_info.falseList;
                    while(temp){
                        printf("%d ", temp->tac->tac_id);
                        temp = temp->next;
                    }
                    printf("\n");
                }

                l_info.falseList = makeList(leftIfFalseCode);
                // backpatch(l_info.trueList, getNextInstruction());
                if(isDebug) printf("[DEBUG] Generated code for left\n");

                // Process right child
                Operand* r_opr1 = NULL;
                BoolExprInfo r_info = {NULL, NULL};
                TAC* rightSubCode = NULL;
                if(node->expr_data.right->type == NODE_EXPR_TERM){
                    attachValueOfExprTerm(node->expr_data.right, &r_opr1);
                }else{
                    rightSubCode = generateCode(node->expr_data.right, &r_info);
                    r_opr1 = makeOperand(ID_REF, rightSubCode->result);
                    if(isDebug) printf("[DEBUG] Right sub expr result %s\n", rightSubCode->result); 
                }
                
                
                
            
                // Generate code for right
                TAC* rightIfCode = createTAC(TAC_IF_FALSE_GOTO, NULL, r_opr1, NULL);
                // TAC* rightGotoCode = createTAC(TAC_GOTO, NULL, NULL, NULL);
                appendTAC(codeList, rightIfCode);
                // appendTAC(codeList, rightGotoCode);

               if(isDebug){
                    printf("[DEBUG] Right sub expr truelist: ");
                    temp = r_info.trueList;
                    while(temp){
                        printf("%d ", temp->tac->tac_id);
                        temp = temp->next;
                    }
                    printf("\n[DEBUG] Right sub expr falselist: ");
                    temp = r_info.falseList;
                    while(temp){
                        printf("%d ", temp->tac->tac_id);
                        temp = temp->next;
                    }
                    printf("\n");
                }

                r_info.trueList = NULL;
                r_info.falseList = makeList(rightIfCode);

                
                if(isDebug) printf("[DEBUG] Generated code for right\n"); 
                

                // if(rightSubCode){
                //     backpatch(l_info.trueList, rightSubCode->tac_id);        // If L is TRUE, evaluate R
                // }else{
                //     backpatch(l_info.trueList, rightIfCode->tac_id);
                // } 
                
                bool_info->trueList = r_info.trueList;    
                bool_info->falseList = merge(l_info.falseList, r_info.falseList);
                if(isDebug){
                    printf("[DEBUG] Cur Node truelist: ");
                    temp = bool_info->trueList;
                    while(temp){
                        printf("%d ", temp->tac->tac_id);
                        temp = temp->next;
                    }
                    printf("\n[DEBUG] Cur Node falselist: ");
                    temp = bool_info->falseList;
                    while(temp){
                        printf("%d ", temp->tac->tac_id);
                        temp = temp->next;
                    }
                    printf("\n");
                }

                 
                if(isDebug) printf("[DEBUG] Generating Code for bool result\n");
                char* result = newTempVar();
                leftIfFalseCode->result = result;

                int val_1 = 1;
                TAC* assignTrue = createTAC(TAC_ASSIGN, result, makeOperand(INT_VAL, &val_1), NULL);
                appendTAC(codeList, assignTrue);
                backpatch(bool_info->trueList, assignTrue->tac_id);
                TAC* skipCode = createTAC(TAC_GOTO, NULL, NULL, NULL);
                appendTAC(codeList, skipCode);
                skipCode->target_jump = getNextInstruction() + 1;
                
                int val_0 = 0;
                TAC* assignFalse = createTAC(TAC_ASSIGN, result, makeOperand(INT_VAL, &val_0), NULL);
                appendTAC(codeList, assignFalse); 
                backpatch(bool_info->falseList, assignFalse->tac_id);

                if(isDebug) printf("[DEBUG] Generated code for bool expr\n");
                return leftIfFalseCode; 

            }      
            else if (strcmp(op, "||") == 0){
                if(isDebug) printf("[DEBUG] Handling logical (||) expression\n");
                //Process left child
                Operand* l_opr1 = NULL; 
                BoolExprInfo l_info = {NULL, NULL};
                TAC* leftSubCode = NULL;
                if(node->expr_data.left->type == NODE_EXPR_TERM){
                    attachValueOfExprTerm(node->expr_data.left, &l_opr1);
                }else{
                    leftSubCode = generateCode(node->expr_data.left, &l_info);
                    l_opr1 = makeOperand(ID_REF, leftSubCode->result); 
                    if(isDebug) printf("[DEBUG] Left sub expr result %s\n", leftSubCode->result);
                } 

                
                // Generate code for left
                TAC* leftIfCode = createTAC(TAC_IF_GOTO, NULL, l_opr1, NULL);
                // TAC* leftGotoCode = createTAC(TAC_GOTO, NULL, NULL, NULL);
                appendTAC(codeList, leftIfCode);
                // appendTAC(codeList, leftGotoCode);

                List* temp = NULL;
                if(isDebug){
                    printf("[DEBUG] Left sub expr truelist: ");
                    temp = l_info.trueList;
                    while(temp){
                        printf("%d ", temp->tac->tac_id);
                        temp = temp->next;
                    }
                    printf("\n[DEBUG] Left sub expr falselist: ");
                    temp = l_info.falseList;
                    while(temp){
                        printf("%d ", temp->tac->tac_id);
                        temp = temp->next;
                    }
                    printf("\n");
                }


                l_info.trueList = makeList(leftIfCode);
                // l_info.falseList = makeList(leftGotoCode);
                // backpatch(l_info.falseList, getNextInstruction());
                if(isDebug) printf("[DEBUG] Generated code for left\n");
                

                // Process right child
                Operand* r_opr1 = NULL;
                BoolExprInfo r_info = {NULL, NULL};
                TAC* rightSubCode = NULL;
                if(node->expr_data.right->type == NODE_EXPR_TERM){
                    attachValueOfExprTerm(node->expr_data.right, &r_opr1);
                }else{
                    rightSubCode = generateCode(node->expr_data.right, &r_info);
                    r_opr1 = makeOperand(ID_REF, rightSubCode->result);
                    if(isDebug) printf("[DEBUG] Right sub expr result %s\n", rightSubCode->result);  
                }

               

                // Generate code for right
                TAC* rightIfCode = createTAC(TAC_IF_FALSE_GOTO, NULL, r_opr1, NULL);
                // TAC* rightGotoCode = createTAC(TAC_GOTO, NULL, NULL, NULL);
                appendTAC(codeList, rightIfCode);
                // appendTAC(codeList, rightGotoCode);


               if(isDebug){
                    printf("[DEBUG] Right sub expr truelist: ");
                    temp = r_info.trueList;
                    while(temp){
                        printf("%d ", temp->tac->tac_id);
                        temp = temp->next;
                    }
                    printf("\n[DEBUG] Right sub expr falselist: ");
                    temp = r_info.falseList;
                    while(temp){
                        printf("%d ", temp->tac->tac_id);
                        temp = temp->next;
                    }
                    printf("\n");
                }



                List* r_trueList = NULL;
                List* r_falseList = makeList(rightIfCode);
                 if(isDebug) printf("[DEBUG] Generated code for right\n"); 

                // if(rightSubCode){
                //     backpatch(l_info.falseList, rightSubCode->tac_id);        // If L is TRUE, evaluate R
                // }else{
                //     backpatch(l_info.falseList, rightIfCode->tac_id);
                // }
                
                bool_info->trueList = l_info.trueList;    
                bool_info->falseList = r_falseList;
                
                if(isDebug){
                    printf("[DEBUG] Cur Node truelist: ");
                    temp = bool_info->trueList;
                    while(temp){
                        printf("%d ", temp->tac->tac_id);
                        temp = temp->next;
                    }
                    printf("\n[DEBUG] Cur Node falselist: ");
                    temp = bool_info->falseList;
                    while(temp){
                        printf("%d ", temp->tac->tac_id);
                        temp = temp->next;
                    }
                    printf("\n");
                }
                 
                if(isDebug) printf("[DEBUG] Generating Code for bool result\n");
                // Generate TAC to store the boolean result in a temporary variable
                char* result = newTempVar();
                leftIfCode->result = result; // To give access to other parent bool

                int val_1 = 1;
                TAC* assignTrue = createTAC(TAC_ASSIGN, result, makeOperand(INT_VAL, &val_1), NULL);
                appendTAC(codeList, assignTrue);
                backpatch(bool_info->trueList, assignTrue->tac_id);
                TAC* skipCode = createTAC(TAC_GOTO, NULL, NULL, NULL);
                appendTAC(codeList, skipCode);
                skipCode->target_jump = getNextInstruction() + 1;
                
                int val_0 = 0;
                TAC* assignFalse = createTAC(TAC_ASSIGN, result, makeOperand(INT_VAL, &val_0), NULL);
                appendTAC(codeList, assignFalse); 
                backpatch(bool_info->falseList, assignFalse->tac_id);

                if(isDebug) printf("[DEBUG] Generated Code for bool result\n");
                return leftIfCode; 

            }; 
            break;
        }
        default:
        fprintf(stderr, "Unsupported binary operator\n");
        exit(1);
    }

    TAC* newTac = createTAC(tac_op, newTempVar(), opr1, opr2);
    appendTAC(codeList, newTac); 

    return newTac;
}



TAC* genCodeForUnaryExpr(ASTNode* node, BoolExprInfo* bool_info){
    if(isDebug) printf("[DEBUG] GenCode for EXPR_UNARY\n");
    if(node->type != NODE_EXPR_UNARY) return NULL;

    if(!(node->expr_data.op)){
        fprintf(stderr, "Operator is NULL\n");
        exit(1); 
    }

    TAC* rhsCode;
    Operand* opr1;

    BoolExprInfo b_info = {NULL, NULL};
 

    const char* node_op = node->expr_data.op;
    TACOp op;
    if(strcmp(node_op, "-") == 0)                op = TAC_NEG;
    else if(strcmp(node_op, "!") == 0){

        if(isDebug) printf("[DEBUG] Handling logical (!) expression\n");
        
        Operand* l_opr1 = NULL;

        BoolExprInfo l_info = {NULL, NULL};
        TAC* leftSubCode = NULL;

        if(node->expr_data.left->type == NODE_EXPR_TERM){
            attachValueOfExprTerm(node->expr_data.left, &l_opr1);
        }else{
            leftSubCode = generateCode(node->expr_data.left, &l_info);
            l_opr1 = makeOperand(ID_REF, leftSubCode->result);
            if(isDebug) printf("[DEBUG] Left sub expr result %s\n", leftSubCode->result); 
        }

        // Generate TAC to store the boolean result in a temporary variable
        char* result = newTempVar();
        // Generate code for left. If true, then result must be false
        TAC* leftIfCode = createTAC(TAC_IF_GOTO, result, l_opr1, NULL); 
        // TAC* leftGotoCode = createTAC(TAC_GOTO, result, NULL, NULL);
        appendTAC(codeList, leftIfCode);
        // appendTAC(codeList, leftGotoCode); 

        List* temp = NULL;
        if(isDebug){
            printf("[DEBUG] Left sub expr truelist: ");
            temp = l_info.trueList;
            while(temp){
                printf("%d ", temp->tac->tac_id);
                temp = temp->next;
            }
            printf("\n[DEBUG] Left sub expr falselist: ");
            temp = l_info.falseList;
            while(temp){
                printf("%d ", temp->tac->tac_id);
                temp = temp->next;
            }
            printf("\n");
        }
            if(isDebug) printf("[DEBUG] Generated code for left\n");

        bool_info->trueList = makeList(leftIfCode);
        bool_info->falseList = NULL;
        if(isDebug){
            printf("[DEBUG] Cur Node truelist: ");
            temp = bool_info->trueList;
            while(temp){
                printf("%d ", temp->tac->tac_id);
                temp = temp->next;
            }
            printf("\n[DEBUG] Cur Node falselist: ");
            temp = bool_info->falseList;
            while(temp){
                printf("%d ", temp->tac->tac_id);
                temp = temp->next;
            }
            printf("\n");
        }

        // backpatch(l_info.trueList, getNextInstruction());

        if(isDebug) printf("[DEBUG] Generating Code for bool result\n");
        int val_1 = 1;
        TAC* assignTrue = createTAC(TAC_ASSIGN, result, makeOperand(INT_VAL, &val_1), NULL);
        appendTAC(codeList, assignTrue);
        // backpatch(bool_info->trueList, assignTrue->tac_id);
        TAC* skipCode = createTAC(TAC_GOTO, NULL, NULL, NULL);
        appendTAC(codeList, skipCode);
        skipCode->target_jump = getNextInstruction() + 1;
        
        int val_0 = 0;
        TAC* assignFalse = createTAC(TAC_ASSIGN, result, makeOperand(INT_VAL, &val_0), NULL);
        appendTAC(codeList, assignFalse); 
        backpatch(bool_info->trueList, assignFalse->tac_id); // If value is true, then make it false for !operator
        
        if(isDebug) printf("[DEBUG] Generated Code for bool result\n");
        return leftIfCode; 

    } 
    else if(strcmp(node_op, "POST_INC") == 0){
        // Assign the original value
        op = TAC_ASSIGN;
        char* result = newTempVar();
        TAC* newTac = createTAC(op, result, opr1, NULL);
        appendTAC(codeList, newTac); 

        // Increment later
        int temp_val = 1;
        Operand* opr2 = makeOperand(INT_VAL, &temp_val);
        TAC* postInc = createTAC(TAC_ADD, opr1->id_ref.name, opr1, opr2);

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
        TAC* postInc = createTAC(TAC_SUB, opr1->id_ref.name, opr1, opr2);
        appendTAC(codeList, postInc);
        return newTac;
    }    
    else if(strcmp(node_op, "PRE_INC") == 0){
        // Increment First
        int temp_val = 1;
        Operand* opr2 = makeOperand(INT_VAL, &temp_val);
        TAC* postInc = createTAC(TAC_ADD, opr1->id_ref.name, opr1, opr2);
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
        TAC* postInc = createTAC(TAC_SUB, opr1->id_ref.name, opr1, opr2);
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

       
    if(node->expr_data.left->type == NODE_EXPR_TERM){
        attachValueOfExprTerm(node->expr_data.left, &opr1);
    }else{
        rhsCode = generateCode(node->expr_data.left, &b_info);
        opr1 = makeOperand(ID_REF,rhsCode->result);
    }  

    char* result = newTempVar();
    TAC* newTac = createTAC(op, result, opr1, NULL);

    appendTAC(codeList, newTac);

    return newTac;
}

TAC* genCodeForIfElse(ASTNode* node, BoolExprInfo* bool_info){
    if(isDebug) printf("[DEBUG] GenCode for IF_ELSE \n");
    if(node->type != NODE_IF_ELSE) return NULL;
    
    BoolExprInfo cond_info = {NULL, NULL};
 
    ASTNode* cond = node->if_else_data.condition->if_cond_data.cond;
    Operand* cond_opr = NULL;
    TAC* cond_code = NULL;

    if(cond->type == NODE_EXPR_TERM){
        attachValueOfExprTerm(cond, &cond_opr);
    }else{
        cond_code = generateCode(cond, &cond_info);
        cond_opr = makeOperand(ID_REF, cond_code->result);
        if(isDebug) printf("[DEBUG] Cond code result %s\n", cond_code->result); 
    } 

    TAC* ifFalseCode = createTAC(TAC_IF_FALSE_GOTO, NULL, cond_opr, NULL);
    appendTAC(codeList, ifFalseCode);

    List* temp = NULL;
    if(isDebug){
        printf("[DEBUG] Cond truelist: ");
        temp = cond_info.trueList;
        while(temp){
            printf("%d ", temp->tac->tac_id);
            temp = temp->next;
        }
        printf("\n[DEBUG] Cond falselist: ");
        temp = cond_info.falseList;
        while(temp){
            printf("%d ", temp->tac->tac_id);
            temp = temp->next;
        }
        printf("\n");
    }

    bool_info->falseList = makeList(ifFalseCode);

    BoolExprInfo b_info = {NULL, NULL}; // Dummy
    TAC* if_branch_code = generateCode(node->if_else_data.if_branch->if_else_branch.branch, &b_info);
    
    TAC* skipCode = NULL;
    // Generate SKIP code if Else part exists
    if(node->if_else_data.else_branch->if_else_branch.branch){
        skipCode = createTAC(TAC_GOTO, NULL, NULL, NULL);
        appendTAC(codeList, skipCode);
        backpatch(bool_info->falseList, getNextInstruction());

        BoolExprInfo b_info = {NULL, NULL}; // Dummy
        TAC* else_branch_code = generateCode(node->if_else_data.else_branch->if_else_branch.branch, &b_info);
        skipCode->target_jump = getNextInstruction(); // Patch the Skip Code with correct jump
    }else{
        backpatch(bool_info->falseList, getNextInstruction()); 
    }

     

    return ifFalseCode;

}


TAC* genCodeForTermExpr(ASTNode* node){
    if(isDebug) printf("[DEBUG] GenCode for TERM EXPR\n");
    if (node->type != NODE_EXPR_TERM) return NULL;

    Operand* opr1 = NULL;
    char* result = newTempVar();

    attachValueOfExprTerm(node->expr_data.left, &opr1);

    TAC* code = createTAC(TAC_ASSIGN, result, opr1, NULL);
    appendTAC(codeList, code);

    return code;

}

// Generate code for an assignment statement
TAC* generateCodeForAssignment(ASTNode* node) {
    if(isDebug) printf("[DEBUG] GenCode for NODE_ASSGN\n");
    if (node->type != NODE_ASSGN) return NULL;
    BoolExprInfo b_info = {NULL, NULL};
    Operand* opr1;
    char* result = strdup(generateScopeSuffixedName(node->assgn_data.left->id_ref_data.name, node->assgn_data.left->id_ref_data.ref->scope->table_id)); 

    if(node->assgn_data.right->type == NODE_EXPR_TERM){
       attachValueOfExprTerm(node->assgn_data.right, &opr1); 
    }else{
       TAC* rhsCode = generateCode(node->assgn_data.right, &b_info);
       opr1 = makeOperand(ID_REF, rhsCode->result); 
    }

    TAC* code = createTAC(TAC_ASSIGN, result, opr1, NULL);
    appendTAC(codeList, code);

    return code;
}

 

TAC* genCodeForVar(ASTNode* node){
    if(isDebug) printf("[DEBUG] GenCode for VAR\n");
    if(node->type != NODE_VAR) return NULL;


    Operand* opr1;
    BoolExprInfo b_info = {NULL, NULL};
    char* result = strdup(generateScopeSuffixedName(node->var_data.id->id_data.sym->name, node->var_data.id->id_data.sym->scope->table_id));

    if(node->var_data.value == NULL){
        opr1 = NULL;
    }else{
        if(node->var_data.value->type == NODE_EXPR_TERM){
            attachValueOfExprTerm(node->var_data.value, &opr1);
        }else{
            TAC* rhsCode = generateCode(node->var_data.value, &b_info);
            opr1 = makeOperand(ID_REF, rhsCode->result);
        }    
    }
    TAC* code = createTAC(TAC_ASSIGN, result, opr1, NULL);
    
    appendTAC(codeList, code);

    return code;
}

TAC* genCodeForVarList(ASTNode* node){
    if(isDebug) printf("[DEBUG] GenCode for VAR_LIST\n");
    if(!node) return NULL;
    if(node->type != NODE_VAR_LIST) return NULL;

    TAC* code_var_list = genCodeForVarList(node->var_list_data.var_list);
    TAC* code_var = genCodeForVar(node->var_list_data.var);
    return code_var;
}


TAC* generateCodeForDecl(ASTNode* node){
    if(isDebug) printf("[DEBUG] GenCode for DECL\n");
    if(node->type != NODE_DECL) return NULL;

    TAC* code_var_list = genCodeForVarList(node->decl_data.var_list);
    return code_var_list;
}

// Main function to generate TAC code for an AST node
TAC* generateCode(ASTNode* node, BoolExprInfo* bool_info) {
    if (!node) return NULL;

    switch (node->type) {
        case NODE_PROGRAM:{
            return generateCode(node->program_data.stmt_list, bool_info);
        }
        case NODE_STMT_LIST:{
            generateCode(node->stmt_list_data.stmt_list, bool_info);
            return generateCode(node->stmt_list_data.stmt, bool_info);
        }
        case NODE_STMT:{
            return generateCode(node->stmt_data.stmt, bool_info);
        }
        case NODE_DECL:{
            return generateCodeForDecl(node);
        }
        case NODE_EXPR_BINARY:{
            return generateCodeForBinaryExpr(node, bool_info);
        }
        case NODE_EXPR_UNARY:
            return genCodeForUnaryExpr(node, bool_info);
        case NODE_EXPR_TERM:
            return genCodeForTermExpr(node); 
        case NODE_ASSGN:
            return generateCodeForAssignment(node);

        case NODE_BLOCK_STMT:
            return generateCode(node->block_stmt_data.stmt_list, bool_info);
            // return generateCodeForAssignment(node);
        case NODE_IF_ELSE:
            return genCodeForIfElse(node, bool_info);
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
    printf("%d : ", instr->tac_id);
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
        case TAC_IF_GOTO:
            printf("IF %s GOTO %d\n", opr1, instr->target_jump); 
            break;
        case TAC_IF_FALSE_GOTO:
            printf("IF FALSE %s GOTO %d\n", opr1, instr->target_jump);
            break;
        case TAC_GOTO:
            printf("GOTO %d\n", instr->target_jump);
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
    TAC* current = codeList->head;
    while (current) {
        printTACInstruction(current);
        current = current->next;
    }
}