#include "icg.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int tempVarCounter = 0;
int labelCounter = 0;
int instructionCounter = 1;
const char* ret_val_var = "ret_val";

#define MAX_FUNCTIONS 100
ASTNode* functionQueue[MAX_FUNCTIONS];
int functionCount = 1;
TACList* funcCalls[INT16_MAX];

bool isDebug = false;
void setICGDebugger(){
    isDebug = true;
}

#define MAX_LOOP_STACK_SIZE 100
LoopInfo loopStack[MAX_LOOP_STACK_SIZE];
int loopStackTop = -1;

void pushLoopInfo(ASTNode* loop_node) {
    if (loopStackTop >= MAX_LOOP_STACK_SIZE - 1) {
        fprintf(stderr, "Loop stack overflow\n");
        exit(1);
    }
    loopStackTop++;
    loopStack[loopStackTop].loop_node = loop_node;
    loopStack[loopStackTop].breakList = NULL;
    loopStack[loopStackTop].continueList = NULL;
    if(isDebug){
        printf("[DEBUG] Pushed loop info to stack. (StkSize: %d, StkTop: %s)\n", 
        loopStackTop, 
        loop_node->type == NODE_FOR ? "FOR LOOP" : loop_node->type == NODE_WHILE ? "WHILE LOOP" : "Unsupported node");
    }
}

void popLoopInfo() {
    if (loopStackTop < 0) {
        fprintf(stderr, "Loop stack underflow\n");
        exit(1);
    }
    loopStackTop--;
    if(isDebug) printf("Popped Loop Info stack\n");
}

LoopInfo* getCurrentLoopInfo() {
    if (loopStackTop < 0) return NULL;
    return &loopStack[loopStackTop];
}


// Function to get the index of the next instruction
int getNextInstruction() {
    return instructionCounter;
}



BoolExprInfo global_bool_info = {NULL, NULL, NULL, NULL, NULL};

TACList* codeList;
FuncQ* funcQ;

// Runner
void startICG(ASTNode* root){
    if(isDebug) printf("[DEBUG] Starting ICG generation...\n");
    codeList = createTACList();
    funcQ = createFuncQ();

    generateCode(root, &global_bool_info);
    TAC* code_end = createTAC(TAC_END, NULL, NULL, NULL);
    appendTAC(codeList, code_end);
    appendComments(code_end, "END OF PROGRAM");

    startICGforFunctions(funcQ);
    if(isDebug) printf("[DEBUG] ICG generation completed.\n");
}

void startICGforFunctions(FuncQ* funcQ){
    if (isDebug) printf("[DEBUG] Starting ICG generation for functions...\n");

    ASTNode* func_decl;
    while ((func_decl = dequeue(funcQ)) != NULL) {
        if (isDebug) printf("[DEBUG] Generating TAC for function...\n");
        genCodeForFuncDecl(func_decl, &global_bool_info);
    }

    if (isDebug) printf("[DEBUG] Completed ICG for all functions.\n");
}


ASTNode* dequeue(FuncQ* funcQ) {
    if (funcQ == NULL || funcQ->head == NULL) return NULL;

    FuncQNode* temp = funcQ->head;
    funcQ->head = funcQ->head->next;

    if (funcQ->head == NULL) {
        funcQ->tail = NULL;
    }

    ASTNode* func_decl = temp->func_decl_node;
    free(temp);  
    return func_decl;
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
        fprintf(stderr, "Memory allocation failed for createTAC()\n");
        exit(1);
    } 
    instr->op = op;
    instr->result = result ? strdup(result) : NULL;
    instr->operand1 = operand1;
    instr->operand2 = operand2;
    instr->next = NULL;
    instr->comments = NULL;
    if(isDebug){
        printf("[DEBUG] Created TAC for operation: %s, Result: %s\n", getOperatorString(op), result ? result : "NULL");
    } 
    return instr;   
}


void appendComments(TAC* instr, const char* new_comment) {
    if (new_comment == NULL || !instr) return;  // Nothing to append
    if (isDebug) printf("[DEBUG] appending comments\n");
    if (instr->comments == NULL) {
        // Allocate and set the new comment if it doesn't exist
        instr->comments = strdup(new_comment);
        if (!instr->comments) {
            fprintf(stderr, "Memory allocation failed for comments\n");
            exit(1);
        }
    } else {
        // Calculate the total length needed for the new string
        size_t existing_length = strlen(instr->comments);
        size_t new_length = strlen(new_comment);
        size_t total_length = existing_length + new_length + 3; // +3 for ", " and '\0'

        // Allocate a new block of memory
        char* updated_comments = malloc(total_length);
        if (!updated_comments) {
            fprintf(stderr, "Memory allocation failed for appending comments\n");
            exit(1);
        }

        // Copy the existing comments and append the new comment
        strcpy(updated_comments, instr->comments);
        strcat(updated_comments, ", ");
        strcat(updated_comments, new_comment);

        // Free the old comments and assign the new string
        free(instr->comments);
        instr->comments = updated_comments;
    }
    if (isDebug) printf("[DEBUG] appending comments: DONE\n");
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
        case TAC_POP_ARG:        return "POP_ARG";
        case TAC_PUSH_ARG:       return "PUSH_ARG";
        case TAC_RETURN:         return "RETURN";
        case TAC_END:            return "END";
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

    if (type != VOID_VAL && val == NULL) {
        fprintf(stderr, "Error: val is NULL for Non void type\n");
        return NULL;
    }

    Operand* opr = (Operand*)malloc(sizeof(Operand));
    if (!opr) {
        fprintf(stderr, "Memory allocation failed for makeOperand\n");
        exit(1);
    }
    
    opr->type = type;
    switch (type) {
        case VOID_VAL: {
            opr->void_val.val = "VOID";
            if (isDebug) printf("[DEBUG] Created VOID operand with value: %d\n", opr->int_val);
            break; 
        }
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
        case POP_ARG:
            opr->pop_stk.argNum = *((int*)val); 
            if (isDebug) printf("[DEBUG] Created POP_ARG operand for arg no: %d\n", opr->pop_stk.argNum); 
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

    if (opr->type == VOID_VAL) {
        val = "VOID";
    } else if (opr->type == INT_VAL) {
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
    } else if (opr->type == POP_ARG) {
        val = (char*)malloc(20);
        if (val) sprintf(val, "%d", opr->pop_stk.argNum);
    }

    if (!val) {
        fprintf(stderr, "Memory allocation failed for getFormattedValueFromOperand()\n");
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

FuncQ* createFuncQ(){
    FuncQ* funcQ = (FuncQ*)malloc(sizeof(FuncQ));
    funcQ->head = NULL;
    funcQ->tail = NULL;
    return funcQ;
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

void appendFuncDecl(FuncQ* funcQ, ASTNode* func_decl) {
    if (funcQ == NULL) {
        fprintf(stderr, "FuncQ is not inited!\n");
        exit(1);  
    }

    if (func_decl->type != NODE_FUNC_DECL) {
        fprintf(stderr, "Cannot append func to funcQ, unsupported node type\n");
        exit(1); 
    }
    
    FuncQNode* node = (FuncQNode*)malloc(sizeof(FuncQNode));
    node->func_decl_node = func_decl;
    node->next = NULL;

    if (!funcQ->head) {
        funcQ->head = node;
    } else {
        funcQ->tail->next = node;
    }
    funcQ->tail = node;
    node->id = functionCount++;
    if (isDebug) 
        printf("[DEBUG] Appended new func decl node %s with id %d\n", 
            func_decl->func_decl_data.id->id_data.sym->name,
            node->id);
}

void appendFuncCallTAC(TAC* tac, int global_func_id) {
    if (isDebug) printf("[DEBUG] Appending func call to %d index\n", global_func_id-1);
    if (global_func_id > MAX_FUNCTIONS) {
        fprintf(stderr, "Cannot append node to funcCallList, index exceeds > 100\n");
        exit(1);
    }

    TACList* list = funcCalls[global_func_id-1];
    if (list == NULL) {
        list = createTACList();
        funcCalls[global_func_id-1] = list;
    } 
    
    if (!list->head) {
        list->head = tac;
    } else {
        list->tail->next = tac;
    }
    list->tail = tac;
    if (isDebug) printf("[DEBUG] Appended! func call to %d index\n", global_func_id-1);
}

void attachValueOfExprTerm(ASTNode* node, Operand** opr){
    if(isDebug) printf("[DEBUG] Extracting val from EXPR_TERM, received type: %s\n", getNodeName(node->type));
    if(node->type != NODE_EXPR_TERM){
        fprintf(stderr, "Unsupported node type %s for attachValueOfExprTerm()\n", getNodeName(node->type));
        exit(1); 
    };
    
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
    
            char* suffixed_name = generateScopeSuffixedName(valNode->id_ref_data.name, valNode->id_ref_data.ref->scope->table_id);
        
            *opr = makeOperand(ID_REF, suffixed_name);
            (*opr)->id_ref.sym = valNode->id_ref_data.ref;
            if (isDebug) printf("[DEBUG] Attached ID_REF value: %s\n", suffixed_name);
        }else {
            fprintf(stderr, "Unsupported node type %s for attachValueOfExprTerm()\n", getNodeName(node->type));
            exit(1); 
        }
}




// Generate code for a binary expression
TAC* generateCodeForBinaryExpr(ASTNode* node, BoolExprInfo* bool_info) {
    if(isDebug) printf("[DEBUG] GenCode for BIN EXPR\n"); 
    
    if (node->type != NODE_EXPR_BINARY) return NULL;

    if(!(node->expr_data.op)){
        fprintf(stderr, "Operator is NULL\n");
        exit(1); 
    }

    const char* op = node->expr_data.op;

    switch (getOpType(op)) {
        case OP_COMP:
        case OP_ARITHMETIC:{  
            BoolExprInfo b_info = {NULL, NULL, NULL, NULL, NULL}; 

            Operand* l_opr1 = NULL; 
            BoolExprInfo l_info = {NULL, NULL, NULL, NULL, NULL};
            TAC* leftSubCode = NULL;
            if(node->expr_data.left->type == NODE_EXPR_TERM){
                attachValueOfExprTerm(node->expr_data.left, &l_opr1);
            }else{
                leftSubCode = generateCode(node->expr_data.left, &l_info);
                const char* l_result = l_info.bool_resut != NULL ? l_info.bool_resut : leftSubCode->result;
                l_opr1 = makeOperand(ID_REF, l_result); 
                if(isDebug) printf("[DEBUG] Left sub expr result %s\n", l_result);
            }

            Operand* r_opr1 = NULL;
            BoolExprInfo r_info = {NULL, NULL};
            TAC* rightSubCode = NULL;
            if(node->expr_data.right->type == NODE_EXPR_TERM){
                attachValueOfExprTerm(node->expr_data.right, &r_opr1);
            }else{
                rightSubCode = generateCode(node->expr_data.right, &r_info);
                const char* r_result = r_info.bool_resut != NULL ? r_info.bool_resut : rightSubCode->result;
                r_opr1 = makeOperand(ID_REF, r_result);
                if(isDebug) printf("[DEBUG] Right sub expr result %s\n", r_result); 
            }

            TACOp tac_op;

            if(strcmp(op, "+") == 0)            tac_op = TAC_ADD;
            else if(strcmp(op, "-") == 0)       tac_op = TAC_SUB;
            else if(strcmp(op, "*") == 0)       tac_op = TAC_MUL;
            else if(strcmp(op, "/") == 0)       tac_op = TAC_DIV;
            else if(strcmp(op, "<") == 0)       tac_op = TAC_LT;
            else if(strcmp(op, ">") == 0)       tac_op = TAC_GT;
            else if(strcmp(op, "<=") == 0)      tac_op = TAC_LEQ;
            else if(strcmp(op, ">=") == 0)      tac_op = TAC_GEQ;
            else if(strcmp(op, "==") == 0)      tac_op = TAC_EQ;
            else if(strcmp(op, "!=") == 0)      tac_op = TAC_NEQ;
            else {
                fprintf(stderr, "Unsupported operator");
                exit(1);
            }
            
            TAC* newTac = createTAC(tac_op, newTempVar(), l_opr1, r_opr1);
            appendTAC(codeList, newTac); 

            return newTac;
        }
        case OP_LOGICAL:{
            if (strcmp(op, "&&") == 0){
                if(isDebug) printf("[DEBUG] Handling logical (&&) expression\n");
                //Process left child
                Operand* l_opr1 = NULL; 
                BoolExprInfo l_info = {NULL, NULL, NULL, NULL, NULL};
                TAC* leftSubCode = NULL;
                if(node->expr_data.left->type == NODE_EXPR_TERM){
                    attachValueOfExprTerm(node->expr_data.left, &l_opr1);
                }else{
                    leftSubCode = generateCode(node->expr_data.left, &l_info);
                    const char* l_result = l_info.bool_resut != NULL ? l_info.bool_resut : leftSubCode->result;
                    l_opr1 = makeOperand(ID_REF, l_result); 
                    if(isDebug) printf("[DEBUG] Left sub expr result %s\n", l_result);
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
                    const char* r_result = r_info.bool_resut != NULL ? r_info.bool_resut : rightSubCode->result;
                    r_opr1 = makeOperand(ID_REF, r_result);
                    if(isDebug) printf("[DEBUG] Right sub expr result %s\n", r_result); 
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
                bool_info->bool_resut = result;
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
                return leftSubCode != NULL ? leftSubCode : leftIfFalseCode ; 

            }      
            else if (strcmp(op, "||") == 0){
                if(isDebug) printf("[DEBUG] Handling logical (||) expression\n");
                //Process left child
                Operand* l_opr1 = NULL; 
                BoolExprInfo l_info = {NULL, NULL, NULL, NULL, NULL};
                TAC* leftSubCode = NULL;
                if(node->expr_data.left->type == NODE_EXPR_TERM){
                    attachValueOfExprTerm(node->expr_data.left, &l_opr1);
                }else{
                    leftSubCode = generateCode(node->expr_data.left, &l_info);
                    const char* l_result = l_info.bool_resut != NULL ? l_info.bool_resut : leftSubCode->result;
                    l_opr1 = makeOperand(ID_REF, l_result); 
                    if(isDebug) printf("[DEBUG] Left sub expr result %s\n", l_result);
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
                    const char* r_result = r_info.bool_resut != NULL ? r_info.bool_resut : rightSubCode->result;
                    r_opr1 = makeOperand(ID_REF, r_result);
                    if(isDebug) printf("[DEBUG] Right sub expr result %s\n", r_result); 
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
                bool_info->bool_resut = result;
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
                return leftSubCode != NULL ? leftSubCode : leftIfCode; 

            }; 
            break;
        }
        default:
        fprintf(stderr, "Unsupported binary operator\n");
        exit(1);
    }



    return NULL;
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

    BoolExprInfo b_info = {NULL, NULL, NULL, NULL, NULL};
 

    const char* node_op = node->expr_data.op;
    TACOp op;
    if(strcmp(node_op, "-") == 0)                op = TAC_NEG;
    else if(strcmp(node_op, "!") == 0){

        if(isDebug) printf("[DEBUG] Handling logical (!) expression\n");
        
        Operand* l_opr1 = NULL;

        BoolExprInfo l_info = {NULL, NULL, NULL, NULL, NULL};
        TAC* leftSubCode = NULL;

        if(node->expr_data.left->type == NODE_EXPR_TERM){
            attachValueOfExprTerm(node->expr_data.left, &l_opr1);
        }else{
            leftSubCode = generateCode(node->expr_data.left, &l_info);
            if(l_info.begin_tac != NULL) leftSubCode = l_info.begin_tac;

            const char* l_result = l_info.bool_resut != NULL ? l_info.bool_resut : leftSubCode->result;
            l_opr1 = makeOperand(ID_REF, l_result); 
            if(isDebug) printf("[DEBUG] Left sub expr result %s\n", l_result);
        }

        // Generate TAC to store the boolean result in a temporary variable
        char* result = newTempVar();
        bool_info->bool_resut = result;
        // Generate code for left. If true, then result must be false
        TAC* leftIfCode = createTAC(TAC_IF_GOTO, result, l_opr1, NULL); 
        appendTAC(codeList, leftIfCode);
        appendComments(leftIfCode, "NOT(!) EXPR");

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

        bool_info->begin_tac = leftSubCode != NULL ? leftSubCode : leftIfCode; 
        bool_info->end_tac = assignFalse;

        return bool_info->begin_tac;

    } 
    else if(strcmp(node_op, "POST_INC") == 0){
        // Assign the original value
        op = TAC_ASSIGN;
        char* result = newTempVar();
        if(node->expr_data.left->type == NODE_EXPR_TERM){
            attachValueOfExprTerm(node->expr_data.left, &opr1);
        }else{
            fprintf(stderr, "Error: Unsupported node for POST_INC\n");
            exit(0);
        }
        TAC* newTac = createTAC(op, result, opr1, NULL);
        appendTAC(codeList, newTac); 

        // Increment later
        int temp_val = 1;
        Operand* opr2 = makeOperand(INT_VAL, &temp_val);
        TAC* postInc = createTAC(TAC_ADD, opr1->id_ref.name, opr1, opr2);
        appendComments(postInc, "POST INC");

        appendTAC(codeList, postInc);
        return newTac;
    }   
    else if(strcmp(node_op, "POST_DEC") == 0){
        // Assign the original value
        if(node->expr_data.left->type == NODE_EXPR_TERM){
            attachValueOfExprTerm(node->expr_data.left, &opr1);
        }else{
            fprintf(stderr, "Error: Unsupported node for POST_DEC\n");
            exit(0);
        }
        op = TAC_ASSIGN;
        char* result = newTempVar();
        TAC* newTac = createTAC(op, result, opr1, NULL);
        appendTAC(codeList, newTac);  

        // Decrement later
        int temp_val = 1;
        Operand* opr2 = makeOperand(INT_VAL, &temp_val);
        TAC* postDec = createTAC(TAC_SUB, opr1->id_ref.name, opr1, opr2);
        appendComments(postDec, "POST DEC");
        appendTAC(codeList, postDec);
        return newTac;
    }    
    else if(strcmp(node_op, "PRE_INC") == 0){
        if(node->expr_data.left->type == NODE_EXPR_TERM){
            attachValueOfExprTerm(node->expr_data.left, &opr1);
        }else{
            fprintf(stderr, "Error: Unsupported node for PRE_INC\n");
            exit(0);
        }
        // Increment First
        int temp_val = 1;
        Operand* opr2 = makeOperand(INT_VAL, &temp_val);
        TAC* preInc = createTAC(TAC_ADD, opr1->id_ref.name, opr1, opr2);
        appendComments(preInc, "PRE INC");
        appendTAC(codeList, preInc);

        // Assign the incremented value
        op = TAC_ASSIGN;
        char* result = newTempVar();
        TAC* newTac = createTAC(op, result, opr1, NULL);
        appendTAC(codeList, newTac);  
        
        return newTac;        
    }
    else if(strcmp(node_op, "PRE_DEC") == 0){
        if(node->expr_data.left->type == NODE_EXPR_TERM){
            attachValueOfExprTerm(node->expr_data.left, &opr1);
        }else{
            fprintf(stderr, "Error: Unsupported node for PRE_DEC\n");
            exit(0);
        }
        // Decrement First
        int temp_val = 1;
        Operand* opr2 = makeOperand(INT_VAL, &temp_val);
        TAC* preDec = createTAC(TAC_SUB, opr1->id_ref.name, opr1, opr2);
        appendComments(preDec, "PRE DEC");
        appendTAC(codeList, preDec);

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
    
    BoolExprInfo cond_info = {NULL, NULL, NULL, NULL, NULL};
 
    ASTNode* cond = node->if_else_data.condition->if_cond_data.cond;
    Operand* cond_opr = NULL;
    TAC* cond_code = NULL;

    if(cond->type == NODE_EXPR_TERM){
        attachValueOfExprTerm(cond, &cond_opr);
    }else{
        cond_code = generateCode(cond, &cond_info);

        if(cond_info.begin_tac != NULL) cond_code = cond_info.begin_tac;
        
        appendComments(cond_code, "IF COND");
        const char* cond_result = cond_info.bool_resut != NULL ? cond_info.bool_resut : cond_code->result;
        cond_opr = makeOperand(ID_REF, cond_result);
        if(isDebug) printf("[DEBUG] Cond code result %s\n", cond_result); 
    } 

    TAC* ifFalseCode = createTAC(TAC_IF_FALSE_GOTO, NULL, cond_opr, NULL);
    appendComments(ifFalseCode, "IF COND CHECK");
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
    appendComments(if_branch_code, "IF BODY"); 
    TAC* skipCode = NULL;
    TAC* else_branch_code = NULL;
    // Generate SKIP code if Else part exists
    if(node->if_else_data.else_branch->if_else_branch.branch){
        skipCode = createTAC(TAC_GOTO, NULL, NULL, NULL); 
        appendComments(skipCode, "IF BODY END");
        appendTAC(codeList, skipCode);
        backpatch(bool_info->falseList, getNextInstruction());

        BoolExprInfo b_info = {NULL, NULL}; // Dummy
        else_branch_code = generateCode(node->if_else_data.else_branch->if_else_branch.branch, &b_info);
        appendComments(else_branch_code, "ELSE BODY");
        skipCode->target_jump = getNextInstruction(); // Patch the Skip Code with correct jump
    }else{
        backpatch(bool_info->falseList, getNextInstruction()); 
    }

    bool_info->begin_tac = cond_code != NULL ? cond_code : ifFalseCode;
    bool_info->end_tac = else_branch_code != NULL ? else_branch_code : if_branch_code;
    // Ideally end_tac must be end_tac of else_branch_code

    return bool_info->begin_tac;

}

TAC* genCodeForBrkContStmts(ASTNode* node){
    TAC* code = NULL;
    if(node->type == NODE_BREAK_STMT){
        LoopInfo* curLoopInfo = getCurrentLoopInfo();

        if(curLoopInfo->loop_node == node->break_continue_stmt_data.associated_loop_node){

            code = createTAC(TAC_GOTO, NULL, NULL, NULL);
            appendComments(code, "BREAK");
            appendTAC(codeList, code);

            if(!curLoopInfo->breakList){
                curLoopInfo->breakList = makeList(code);
            }else{
                curLoopInfo->breakList = merge(curLoopInfo->breakList, makeList(code));
            }
        }  
    }else if(node->type == NODE_CONTINUE_STMT){
        LoopInfo* curLoopInfo = getCurrentLoopInfo();

        if(curLoopInfo->loop_node == node->break_continue_stmt_data.associated_loop_node){

            code = createTAC(TAC_GOTO, NULL, NULL, NULL);
            appendComments(code, "CONTINUE");
            appendTAC(codeList, code);

            if(!curLoopInfo->continueList){
                curLoopInfo->continueList = makeList(code);
            }else{
                curLoopInfo->continueList = merge(curLoopInfo->continueList, makeList(code));
            }
        }   
    }

    return code;
}

TAC* genCodeForFORLoop(ASTNode* node, BoolExprInfo* bool_info){
    if(isDebug) printf("[DEBUG] GenCode for FOR LOOP\n");
    if(node->type != NODE_FOR) return NULL;

    // For initialization
    if(isDebug) printf("[DEBUG] GenCode for FOR INIT\n");
    BoolExprInfo init_info = {NULL, NULL, NULL, NULL, NULL};
    TAC* init_code = generateCode(node->for_data.init->for_init_data.init, &init_info);
    // If the init_code is expr comma list
    if(init_info.begin_tac != NULL){
        init_code = init_info.begin_tac;
    }
    if(init_code != NULL) appendComments(init_code, "FOR INIT");

    // For condition
    BoolExprInfo cond_info = {NULL, NULL, NULL, NULL, NULL};
    ASTNode* cond = node->for_data.condition->for_cond_data.cond; 
    Operand* cond_opr = NULL;
    TAC* cond_code = NULL;

   
    if(cond != NULL && cond->type == NODE_EXPR_TERM){
        attachValueOfExprTerm(cond, &cond_opr);
    }else{
        // For cond will be expr_list

        cond_code = generateCode(cond, &cond_info);
        if(cond_code != NULL){
            if(cond_info.begin_tac != NULL) cond_code = cond_info.begin_tac;
            appendComments(cond_code, "FOR COND");

            const char* cond_result = cond_info.bool_resut != NULL ? cond_info.bool_resut : cond_code->result;
            cond_opr = makeOperand(ID_REF, cond_result);
            if(isDebug) printf("[DEBUG] Cond code result %s\n", cond_result);
        }
        
    } 

    TAC* ifFalseCode = NULL;
    if(cond_code != NULL){
        ifFalseCode = createTAC(TAC_IF_FALSE_GOTO, NULL, cond_opr, NULL);
    }else{
        // Generate a IF FALSE code with a cond always true by default if cond_code is NULL
        int true_val = 1; 
        ifFalseCode = createTAC(TAC_IF_FALSE_GOTO, NULL, makeOperand(INT_VAL, &true_val), NULL);
    }
    
    appendComments(ifFalseCode, "CHECK FOR COND");
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

    if(ifFalseCode != NULL)
        bool_info->falseList = makeList(ifFalseCode);

    pushLoopInfo(node);
    LoopInfo* loop_info = getCurrentLoopInfo();
    if(!loop_info){
        fprintf(stderr, "Loop Info stack is NULL");
        exit(0);
    }
    
    // For body
    if(isDebug) printf("[DEBUG] Generating code for FOR BODY\n");
    TAC* for_body_code = NULL;
    BoolExprInfo b_info = {NULL, NULL, NULL, NULL, NULL};
    if(node->for_data.body){
        for_body_code = generateCode(node->for_data.body->for_body_data.body, &b_info);
    }

    
    // For updation
    if(isDebug) printf("[DEBUG] Generating code for FOR UPDATION\n");
    BoolExprInfo updation_info = {NULL, NULL, NULL, NULL, NULL};
    TAC* for_updation_code = generateCode(node->for_data.updation->for_updation_data.updation, &updation_info);
    // If updation is a expr comma list
    if(updation_info.begin_tac != NULL){
        for_updation_code = updation_info.begin_tac;
    }
    appendComments(for_updation_code, "FOR UPDATION");

    TAC* goBackCode = createTAC(TAC_GOTO, NULL, NULL, NULL);
    appendComments(goBackCode, "FOR LOOP END");
    appendTAC(codeList, goBackCode);
    goBackCode->target_jump = cond_code != NULL ? cond_code->tac_id : ifFalseCode->tac_id;


    backpatch(loop_info->continueList, goBackCode->target_jump);
    

    bool_info->falseList = merge(bool_info->falseList, loop_info->breakList);
    backpatch(bool_info->falseList, getNextInstruction());
    appendComments(ifFalseCode->next, "FOR LOOP BODY");
    popLoopInfo();

    TAC* begin_tac = NULL;

    if(init_code != NULL)               begin_tac = init_code;
    else if(cond_code != NULL)          begin_tac = cond_code;
    else if(ifFalseCode != NULL)        begin_tac = ifFalseCode;
    else if(for_body_code != NULL)      begin_tac = for_body_code;
    else if(for_updation_code != NULL)  begin_tac = for_updation_code;

    appendComments(begin_tac, "FOR LOOP START");
    
    bool_info->begin_tac = begin_tac;
    bool_info->end_tac = goBackCode;

    return begin_tac;
}


TAC* genCodeForWhileLoop(ASTNode* node, BoolExprInfo* bool_info){
    if(isDebug) printf("[DEBUG] GenCode for WHILE LOOP\n");
    if(node->type != NODE_WHILE) return NULL;
    
    BoolExprInfo cond_info = {NULL, NULL, NULL, NULL, NULL};
 
    ASTNode* cond = node->while_data.condition->while_cond_data.cond;
    Operand* cond_opr = NULL;
    TAC* cond_code = NULL;

    if(cond->type == NODE_EXPR_TERM){
        attachValueOfExprTerm(cond, &cond_opr);
    }else{
        cond_code = generateCode(cond, &cond_info);
        appendComments(cond_code, "WHILE COND");
        const char* cond_result = cond_info.bool_resut != NULL ? cond_info.bool_resut : cond_code->result;
        cond_opr = makeOperand(ID_REF, cond_result);
        if(isDebug) printf("[DEBUG] Cond code result %s\n", cond_result);
    } 

    TAC* ifFalseCode = createTAC(TAC_IF_FALSE_GOTO, NULL, cond_opr, NULL);
    appendComments(ifFalseCode, "CHECK WHILE COND");
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

    BoolExprInfo b_info = {NULL, NULL, NULL, NULL, NULL}; // Dummy
    
    pushLoopInfo(node);
    LoopInfo* loop_info = getCurrentLoopInfo();
    if(!loop_info){
        fprintf(stderr, "Loop Info stack is NULL");
        exit(0);
    }
    

    TAC* while_body_code = NULL;
    if(node->while_data.while_body){
        while_body_code = generateCode(node->while_data.while_body->while_body_data.body, &b_info);
    }
    
    

    TAC* goBackCode = createTAC(TAC_GOTO, NULL, NULL, NULL);
    goBackCode->target_jump = cond_code != NULL ? cond_code->tac_id : ifFalseCode->tac_id;
    appendComments(goBackCode, "WHILE LOOP END");
    appendTAC(codeList, goBackCode);

    if(cond_code){
        backpatch(loop_info->continueList, cond_code->tac_id);
    }else{
        backpatch(loop_info->continueList, ifFalseCode->tac_id);
    }
     
    bool_info->falseList = merge(bool_info->falseList, loop_info->breakList);
    backpatch(bool_info->falseList, getNextInstruction());
    appendComments(ifFalseCode->next, "WHILE BODY");
    popLoopInfo();

    bool_info->begin_tac = cond_code != NULL ? cond_code : ifFalseCode;
    bool_info->end_tac = goBackCode;

    return bool_info->begin_tac; 
}

TAC* genCodeForTermExpr(ASTNode* node){
    if(isDebug) printf("[DEBUG] genCodeForTermExpr() for node type: %s\n", getNodeName(node->type));
    if (node->type != NODE_EXPR_TERM) return NULL;

    Operand* opr1 = NULL;
    char* result = newTempVar();

    attachValueOfExprTerm(node, &opr1);

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
            if(isDebug) printf("[DEBUG] Rhs result: %s\n", rhsCode->result);
            opr1 = makeOperand(ID_REF, rhsCode->result);
        }    
    }
    TAC* code = createTAC(TAC_ASSIGN, result, opr1, NULL);
    
    appendTAC(codeList, code);

    return code;
}

TAC* genCodeForReturn(ASTNode* node){
    if(isDebug) printf("[DEBUG] genCodeForReturn() for node type: %s\n", getNodeName(node->type));
    if(node->type != NODE_RETURN) {
        fprintf(stderr, "Unsupported AST node type %s for genCodeForReturn()\n", getNodeName(node->type));
        exit(1);
    }  
    BoolExprInfo b_info = {NULL, NULL};
    TAC* ret_val = NULL;
    Operand* opr1;
    if(node->return_data.return_value) {
        ret_val = generateCode(node->return_data.return_value, &b_info);
        opr1 = makeOperand(ID_REF, ret_val->result);
    }else{
        opr1 = makeOperand(VOID_VAL, NULL); 
    }   

    char* result = "ret_val";
    
    TAC* code_ret_val = createTAC(TAC_ASSIGN, result, opr1, NULL);
    appendTAC(codeList, code_ret_val);
    TAC* code_ret = createTAC(TAC_RETURN, NULL, NULL, NULL);
    appendTAC(codeList, code_ret);
    return ret_val ? ret_val : code_ret_val;
}

TAC* genCodeForParam(ASTNode* node, int argNo){
    if(isDebug) printf("[DEBUG] GenCode for PARAM\n");
    if(node->type != NODE_PARAM) return NULL; 

    char* result = strdup(
        generateScopeSuffixedName(
            node->param_data.id->id_data.sym->name, 
            node->param_data.id->id_data.sym->scope->table_id
        )
    );

    int val = argNo;
    Operand* pop_arg =  makeOperand(POP_ARG, &val); 
    TAC* code = createTAC(TAC_POP_ARG, result, pop_arg, NULL);
    appendTAC(codeList, code);

    return code;
}

TAC* genCodeForArg(ASTNode* node, int argNo){
    if(!node) return NULL;
    if(isDebug) printf("[DEBUG] genCodeForArg() for node type %s\n", getNodeName(node->type)); 
    if(node->type != NODE_ARG){
        fprintf(stderr, "Unsupported node type %s for genCodeForArg(). Expected NODE_ARG\n", getNodeName(node->type)); 
        exit(1);
    }
    BoolExprInfo b_info = {NULL, NULL};
    TAC* code_arg = generateCode(node->arg_data.arg, &b_info);
    Operand* opr1 = makeOperand(ID_REF, code_arg->result);
    TAC* code_push = createTAC(TAC_PUSH_ARG, NULL, opr1, NULL); 
    appendTAC(codeList, code_push);
    return code_arg;
}

TAC* genCodeForVarList(ASTNode* node){
    if(!node) return NULL;
    if(isDebug) printf("[DEBUG] GenCode for VAR_LIST\n");
    if(node->type != NODE_VAR_LIST) return NULL;

    TAC* code_var_list = genCodeForVarList(node->var_list_data.var_list);
    TAC* code_var = genCodeForVar(node->var_list_data.var);
    return code_var;
}

TAC* genCodeForParamList(ASTNode* node, int argNum){
    if(!node) return NULL;
    if(isDebug) printf("[DEBUG] GenCode for PARAM_LIST\n"); 
    if(node->type != NODE_PARAM_LIST) return NULL; 

    TAC* code_param = genCodeForParam(node->param_data.id, argNum);
    TAC* code_param_list = genCodeForParamList(node->param_list_data.param_list, argNum - 1);
    return code_param;
}

TAC* genCodeForArgList(ASTNode* node, int argNum){
    if(!node) return NULL;
    if(isDebug) printf("[DEBUG] genCodeForArgList() for node type %s\n", getNodeName(node->type)); 
    if(node->type != NODE_ARG_LIST){
        fprintf(stderr, "Unsupported node type %s for genCodeForArgList()\n", getNodeName(node->type)); 
        exit(1);
    } 

    TAC* code_arg_list = genCodeForArgList(node->arg_list_data.arg_list, argNum - 1);
    TAC* code_arg = genCodeForArg(node->arg_list_data.arg, argNum);
    return code_arg_list;
}


TAC* generateCodeForDecl(ASTNode* node){
    if(isDebug) printf("[DEBUG] GenCode for DECL\n");
    if(node->type != NODE_DECL) return NULL;

    TAC* code_var_list = genCodeForVarList(node->decl_data.var_list);
    return code_var_list;
}

TAC* genCodeForFuncDecl(ASTNode* node, BoolExprInfo* bool_info){
    if(isDebug) printf("[DEBUG] GenCode for FUNC_DECL\n");
    if(node->type != NODE_FUNC_DECL) return NULL;

    // generate TAC for function headers
    ASTNode* paramList = node->func_decl_data.params;
    int paramCnt = node->func_decl_data.param_count;

    TAC* code_param_list = genCodeForParamList(paramList, paramCnt);
    // generate TAC for function body using normal generation
    TAC* code_body = generateCode(node->func_decl_data.body, bool_info);
    
    // Add implicit return with default value
    if(codeList->tail->op != TAC_RETURN){        
        Operand* opr1 = makeOperand(VOID_VAL, NULL);
        TAC* code_ret_val = createTAC(TAC_ASSIGN, (char*)ret_val_var, opr1, NULL);
        TAC* code_end = createTAC(TAC_RETURN, NULL, NULL, NULL);
        appendTAC(codeList, code_ret_val);
        appendTAC(codeList, code_end);
        appendComments(code_end, "IMPLICIT RETURN");
    }

    TACList* list = funcCalls[node->func_decl_data.global_id - 1];
    
    TAC* funCallNode = list == NULL ? NULL : list->head;

    if (isDebug) {
        printf("[DEBUG] Backpatching fun calls for Func: %s, id: %d. Func Call List Null? = %d\n", node->func_decl_data.id->id_data.sym->name, node->func_decl_data.global_id, list == NULL);
        printf("[DEBUG] Code param list null? = %d\n", code_param_list == NULL);
    }
        
    if (code_param_list != NULL) {
        while(funCallNode != NULL && funCallNode->op == TAC_CALL){
            funCallNode->target_jump = code_param_list->tac_id;
            funCallNode = funCallNode->next;
        }

        appendComments(code_param_list, "FUNC START");
        appendComments(code_param_list, node->func_decl_data.id->id_data.sym->name);
    } else {
        while(funCallNode != NULL && funCallNode->op == TAC_CALL){
            funCallNode->target_jump = code_body->tac_id;
            funCallNode = funCallNode->next;
        }

        appendComments(code_body, "FUNC START");
        appendComments(code_body, node->func_decl_data.id->id_data.sym->name); 
    }
    
    appendComments(codeList->tail, "FUNC END");
    appendComments(codeList->tail, node->func_decl_data.id->id_data.sym->name);
    return code_param_list;
}

TAC* genCodeForFuncCall(ASTNode* node){
    if(isDebug) printf("[DEBUG] genCodeForFuncCall() for node type: %s\n", getNodeName(node->type));
    if(node->type != NODE_FUNC_CALL){
        fprintf(stderr, "Unsupported node type %s for genCodeForFuncCall\n", getNodeName(node->type));
        exit(1);
    } 

    ASTNode* argList = node->func_call_data.arg_list;
    int argCnt = node->func_call_data.arg_count;

    TAC* code_arg_list = genCodeForArgList(argList, argCnt);
    appendComments(code_arg_list, "PUSH ARGS");

    Operand* opr1 = makeOperand(ID_REF, node->func_call_data.id->id_ref_data.name);
    char* temp_var = newTempVar();
    TAC* code_func_call = createTAC(TAC_CALL, temp_var, opr1, NULL);
    appendTAC(codeList, code_func_call);
    appendComments(code_func_call, "FUNC CALL BEGIN");
    appendFuncCallTAC(code_func_call, node->func_call_data.id->id_ref_data.ref->func_node->func_decl_data.global_id);
    
    Operand* ret_val = makeOperand(ID_REF, ret_val_var);
    TAC* code_ret_val = createTAC(TAC_ASSIGN, temp_var, ret_val, NULL);
    appendTAC(codeList, code_ret_val);
    appendComments(code_ret_val, "FUCN CALL END, SAVE RET VAL");

    return code_arg_list ? code_arg_list : code_func_call;
}


// Main function to generate TAC code for an AST node
TAC* generateCode(ASTNode* node, BoolExprInfo* bool_info) {
    if (!node) return NULL;
    if (isDebug) printf("[DEBUG] generateCode() for %s\n", getNodeName(node->type));

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
        case NODE_EXPR_COMMA_LIST:{
            generateCode(node->expr_comma_list_data.expr_comma_list, bool_info);
            TAC* code = generateCode(node->expr_comma_list_data.expr_comma_list_item, bool_info);
            if(bool_info->begin_tac == NULL) bool_info->begin_tac = code;
            bool_info->end_tac = code;
            return code;
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
        case NODE_WHILE:
            return genCodeForWhileLoop(node, bool_info);
        case NODE_FOR:
            return genCodeForFORLoop(node, bool_info);
        case NODE_BREAK_STMT:
        case NODE_CONTINUE_STMT:
            return genCodeForBrkContStmts(node);
        case NODE_RETURN:
            return genCodeForReturn(node);
        case NODE_FUNC_DECL:{
            if (functionCount >= MAX_FUNCTIONS) {
                fprintf(stderr, "Too many functions declared\n");
                exit(1);
            }
            appendFuncDecl(funcQ, node);
            return NULL;  // Skip code generation now
        }
        case NODE_FUNC_BODY:
            return generateCode(node->block_stmt_data.stmt_list, bool_info);
        case NODE_FUNC_CALL:
            return genCodeForFuncCall(node);
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
    char instr_buffer[256];
    switch (instr->op) {
        case TAC_ASSIGN:
            sprintf(instr_buffer, "%s = %s", instr->result, opr1);
            break;
        case TAC_POP_ARG:
            sprintf(instr_buffer, "%s = POP(%s)", instr->result, opr1);
            break;
        case TAC_PUSH_ARG:
            sprintf(instr_buffer, "PUSH(%s)", opr1); 
            break;
        case TAC_CALL:
            sprintf(instr_buffer, "CALL %d (%s)", instr->target_jump, opr1);
            break;
        case TAC_ADD:
            sprintf(instr_buffer, "%s = %s + %s", instr->result, opr1, opr2);
            break;
        case TAC_SUB:
            sprintf(instr_buffer, "%s = %s - %s", instr->result, opr1, opr2);
            break;
        case TAC_MUL:
            sprintf(instr_buffer, "%s = %s * %s", instr->result, opr1, opr2);
            break;
        case TAC_DIV:
            sprintf(instr_buffer, "%s = %s / %s", instr->result, opr1, opr2);
            break;
        case TAC_EQ:
            sprintf(instr_buffer, "%s = %s == %s", instr->result, opr1, opr2);
            break;
        case TAC_NEQ:
            sprintf(instr_buffer, "%s = %s != %s", instr->result, opr1, opr2);
            break;
        case TAC_GT:
            sprintf(instr_buffer, "%s = %s > %s", instr->result, opr1, opr2);
            break;
        case TAC_LT:
            sprintf(instr_buffer, "%s = %s < %s", instr->result, opr1, opr2);
            break;
        case TAC_GEQ:
            sprintf(instr_buffer, "%s = %s >= %s", instr->result, opr1, opr2);
            break;
        case TAC_LEQ:
            sprintf(instr_buffer, "%s = %s <= %s", instr->result, opr1, opr2);
            break;
        case TAC_NEG:
            sprintf(instr_buffer, "%s = -%s", instr->result, opr1);
            break;        
        case TAC_IF_GOTO:
            sprintf(instr_buffer, "IF %s GOTO %d", opr1, instr->target_jump); 
            break;
        case TAC_IF_FALSE_GOTO:
            sprintf(instr_buffer, "IF FALSE %s GOTO %d", opr1, instr->target_jump);
            break;
        case TAC_GOTO:
            sprintf(instr_buffer, "GOTO %d", instr->target_jump);
            break;
        case TAC_RETURN:
            sprintf(instr_buffer, "RETURN");
            break;
        case TAC_END:
            sprintf(instr_buffer, "END");
            break;
        default:
            fprintf(stderr, "Unknown TAC operation\n");
            break;
    }

    if (instr->comments != NULL) {
        // Allocate memory for the complete comment string with prefix
        char* comments = malloc(strlen(instr->comments) + 4); // 4 = 2 for "//", 1 for space, 1 for '\0'
        if (!comments) {
            fprintf(stderr, "Memory allocation failed for comments\n");
            exit(1);
        }

        // Format the comment string
        sprintf(comments, "// %s", instr->comments);
        printf("%d : %-40s %s\n", instr->tac_id, instr_buffer, comments);

        // Free the allocated memory for comments
        free(comments);
    } else {
        printf("%d : %s\n", instr->tac_id, instr_buffer); 
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