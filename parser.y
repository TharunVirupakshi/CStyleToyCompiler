%{
#include "symTable.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>
#include "semantic.h"

#define GLOBAL "global"
#define FUNCTION "function"
#define BLOCK "block"  

bool isSemanticError = false;

// Declare extern to access the variables defined in lexer
extern int cur_line;
extern int cur_char;

void yyerror(const char* s);
int yylex(void);

// Global Scope
SymbolTable* symTable;
// Current Scope
SymbolTable* currentScope;

const char *folderPathForAST_Vis = "AST_Vis"; 
int scopeDepth = 0;

ASTNode* root;
ASTNode* createProgramNode(ASTNode* stmt_list);
ASTNode* createStmtListNode(ASTNode* stmtList, ASTNode* stmt);
ASTNode* createBlockStmtNode(ASTNode* stmt_list);
ASTNode* createReturnNode(ASTNode* return_value);
ASTNode* createBinaryExpNode(ASTNode* left, ASTNode* right, const char* op);
ASTNode* createUnaryExpNode(ASTNode* left, const char* op);
ASTNode* createTermExpNode(ASTNode* term);
ASTNode* createIdentifierNode(const char* id, char* type);
ASTNode* createIdRefNode(const char* id);
ASTNode* createIntLiteralNode(int i);
ASTNode* createCharLiteralNode(char c);
ASTNode* createStrLiteralNode(const char* s);
ASTNode* createTypeNode(char* type);
ASTNode* createDeclNode(ASTNode* type_spec, ASTNode* var_list);
void setVarListType(ASTNode* typeNode, ASTNode* var_list);
ASTNode* createVarNode(const char* id);
ASTNode* createVarAssgnNode(const char* id, ASTNode* value);
ASTNode* createVarListNode(ASTNode* var_list, ASTNode* var);
ASTNode* createAssgnNode(const char* id, ASTNode* value);
ASTNode* createIfElseNode(ASTNode* cond, ASTNode* if_branch, ASTNode* else_branch);
ASTNode* createIfNode(ASTNode* cond, ASTNode* if_branch);
ASTNode* createWhileNode(ASTNode* cond, ASTNode* body);
ASTNode* createForNode(ASTNode* init, ASTNode* cond, ASTNode* updation, ASTNode* body);
ASTNode* createCommaExprList(ASTNode* expr_list, ASTNode* expr_list_item);
ASTNode* createFucnIdNode(const char* id, ASTNode* type_spec);
ASTNode* createFuncDeclNode(ASTNode* type_spec, ASTNode* id, ASTNode* params, ASTNode* body);
ASTNode* createParamsListNode(ASTNode* parmas_list, ASTNode* param);
ASTNode* createParamNode(ASTNode* type_spec, const char* id);
ASTNode* createFuncCallNode(const char* id, ASTNode* arg_list);
ASTNode* createArgListNode(ASTNode* arg_list, ASTNode* arg);
ASTNode* createArgNode(ASTNode* arg);
%}



%union {
    int ival;
    const char* strval;     
    ASTNode* node;

    struct {
        ASTNode* type;
        ASTNode* id;
    } func_header_data;

    struct {
        ASTNode* else_body;
    } cond_footer_data;

}

%token <strval> ID STR_LITERAL
%token <ival> INT_LITERAL CHAR_LITERAL
%token IF ELSE WHILE FOR RETURN INT FLOAT CHAR VOID STRING
%token PLUS MINUS MULT DIV INC DEC
%token EQ NEQ LEQ GEQ LT GT ASSIGN
%token AND OR NOT

%right ASSIGN NOT
%left EQ NEQ
%left LT GT LEQ GEQ
%left AND
%left OR
%left PLUS MINUS
%left MULT DIV
%nonassoc UNARY

%type <node> program type_spec expr expr_list stmt block_stmt decl_stmt decl cond_stmt loop_stmt 
%type <node> assgn_expr func_call func_call_stmt func_decl stmt_list ret_stmt assgn_stmt for_init for_expr
%type <node> var_list var param params arg_list expr_stmt expr_list_item body block_stmt_without_scope
%type <func_header_data> func_header
%type <cond_footer_data> else_part

%%

// Program structure
program: 
    stmt_list  { 
        $$ = createProgramNode($1); root = $$; 
    }
    ;

// List of statements
stmt_list:
    stmt_list stmt      { $$ = createStmtListNode($1, $2); } 
    |                   { $$ = NULL;}  
    ;
            
// Different types of statements
stmt:
    decl_stmt           { $$ = $1; }
    | assgn_stmt        { $$ = $1; }
    | expr_stmt         { $$ = $1; }
    | cond_stmt         { $$ = $1; }         
    | block_stmt        { $$ = $1; }    
    | loop_stmt         { $$ = $1; }
    | ret_stmt          { $$ = $1; }
    | func_decl         { $$ = $1; }
    | func_call_stmt    { $$ = $1; }
    | ';'               { $$ = NULL; }
    ;



// Return statement
ret_stmt:
    RETURN expr ';'     { $$ = createReturnNode($2); }   
    | RETURN ';'        { $$ = createReturnNode(NULL);}
    ;



// Function call statement
func_call_stmt:
    func_call ';'       { $$ = $1; }
    ;

// Assignment statement
assgn_stmt:
    assgn_expr ';'      { $$ = $1; }
    ;
assgn_expr:
    ID ASSIGN expr              { $$ = createAssgnNode($1, $3); } 
    | ID ASSIGN STR_LITERAL     { $$ = createAssgnNode($1, createStrLiteralNode($3)); }
    ;

// Block statement
block_stmt:
    '{' {currentScope = enterScope("block", currentScope);} stmt_list '}'   { $$ = createBlockStmtNode($3); currentScope = exitScope(currentScope); } 
    ;
block_stmt_without_scope: 
    '{' stmt_list '}'   { $$ = createBlockStmtNode($2); }
    ;

/*
    Generic body where the first block stmt will not have its own scope
*/
body:
    block_stmt_without_scope    { $$ = $1; }
    | decl_stmt                 { $$ = $1; }
    | assgn_stmt                { $$ = $1; }
    | expr_stmt                 { $$ = $1; }
    | cond_stmt                 { $$ = $1; }         
    | loop_stmt                 { $$ = $1; }
    | ret_stmt                  { $$ = $1; }
    | func_decl                 { $$ = $1; }
    | func_call_stmt            { $$ = $1; }
    | ';'                       { $$ = NULL; }

// Conditional statement
cond_stmt:
    IF '(' expr ')' {
        currentScope = enterScope("block (if)", currentScope);
    } body {
        currentScope = exitScope(currentScope);
    } else_part { 
        $$ = createIfElseNode($3, $6, $8.else_body); 
    } 

else_part:
    ELSE {
        currentScope = enterScope("block (else)", currentScope);
    } body { 
        $$.else_body = $3; 
        currentScope = exitScope(currentScope); 
    }
    |           { $$.else_body = NULL; }
    ;



// Loop statements
loop_stmt:
    WHILE '(' expr ')' {
        currentScope = enterScope("block (while)", currentScope);
    } body { 
        $$ = createWhileNode($3, $6); 
        currentScope = exitScope(currentScope); 
    } 
    | FOR '(' {currentScope = enterScope("block (for)", currentScope);} for_init ';' for_expr ';' for_expr ')' body {
        $$ = createForNode($4, $6, $8, $10);
        currentScope = exitScope(currentScope); 
    }
    ; 

for_init:
    decl            { $$ = $1; }
    | expr_list     { $$ = $1; }
    |               { $$ = NULL; }
    ;

// Expression list for for loop
expr_list:
    expr_list ',' expr_list_item  { $$ = createCommaExprList($1, $3); }
    | expr_list_item              { $$ = createCommaExprList(NULL, $1); }
    ;
expr_list_item:
    assgn_expr      { $$ = $1; }                   
    | expr          { $$ = $1; }
    ;

for_expr:
    expr_list       { $$ = $1; }
    |               { $$ = NULL; }             
    ;



// Declaration statement
decl_stmt:
    decl ';'        { $$ = $1; } 
    ;

// Declaration
decl:
    type_spec var_list  { 
        setVarListType($1, $2); 
        $$ = createDeclNode($1, $2); 
    }
    ;

// Type specifications
type_spec:
    INT       { $$ = createTypeNode("int"); }               
    | CHAR    { $$ = createTypeNode("char"); }     
    | FLOAT   { $$ = createTypeNode("float"); }          
    | STRING  { $$ = createTypeNode("string"); }           
    ;

// Variable list for declarations
var_list:
    var_list ',' var    { $$ = createVarListNode($1, $3); }
    | var               { $$ = createVarListNode(NULL, $1); }
    ;             

// Variable definitions
var:
    ID                          { $$ = createVarNode($1); } 
    | ID ASSIGN expr            { $$ = createVarAssgnNode($1, $3); }
    | ID ASSIGN STR_LITERAL     { $$ = createVarAssgnNode($1, createStrLiteralNode($3)); }
    ;

// Function declarations
func_decl:
    func_header params ')' body { 
            $$ = createFuncDeclNode($1.type, $1.id, $2, $4);
            currentScope = exitScope(currentScope);
    }
    ;

func_header:
    type_spec ID '(' {
        $$.type = $1;
        $$.id = createFucnIdNode($2, $1);
        currentScope = enterScope((char*)$2, currentScope);
    }
    | VOID ID '(' {
        $$.type = createTypeNode("void");
        $$.id = createFucnIdNode($2, $$.type);
        currentScope = enterScope((char*)$2, currentScope); 
    }
    ;

// Function calls
func_call:
    ID '(' arg_list ')'         { $$ = createFuncCallNode($1, $3); }
    ;

// Argument list for function calls
arg_list:
    arg_list ',' expr           { $$ = createArgListNode($1, $3); } 
    | expr                      { $$ = createArgListNode(NULL, $1); }
    |                           { $$ = NULL; }
    ;

// Parameter list for function declarations
params:
    params ',' param    { $$ = createParamsListNode($1, $3); }
    | param             { $$ = createParamsListNode(NULL, $1); }
    |                   { $$ = NULL; }
    ;

// Parameter definition
param:
    type_spec ID        { $$ = createParamNode($1, $2); } 
    ;


// Expression statement
expr_stmt:
    expr ';' { $$ = $1; }
    ;

// Expressions
expr:
    expr PLUS expr              { $$ = createBinaryExpNode($1, $3, "+"); }              
    | expr MINUS expr           { $$ = createBinaryExpNode($1, $3, "-"); }
    | expr MULT expr            { $$ = createBinaryExpNode($1, $3, "*"); }
    | expr DIV expr             { $$ = createBinaryExpNode($1, $3, "/"); }
    | expr EQ expr              { $$ = createBinaryExpNode($1, $3, "=="); }
    | expr NEQ expr             { $$ = createBinaryExpNode($1, $3, "!="); }
    | expr LT expr              { $$ = createBinaryExpNode($1, $3, "<"); }
    | expr GT expr              { $$ = createBinaryExpNode($1, $3, ">"); }
    | expr LEQ expr             { $$ = createBinaryExpNode($1, $3, "<="); }
    | expr GEQ expr             { $$ = createBinaryExpNode($1, $3, ">="); }
    | expr AND expr             { $$ = createBinaryExpNode($1, $3, "&&"); }
    | expr OR expr              { $$ = createBinaryExpNode($1, $3, "||"); }
    | NOT expr                  { $$ = createUnaryExpNode($2, "!"); }
    | MINUS expr %prec UNARY    { $$ = createUnaryExpNode($2, "-"); }
    | INC expr   %prec UNARY    { $$ = createUnaryExpNode($2, "PRE_INC"); }
    | DEC expr   %prec UNARY    { $$ = createUnaryExpNode($2, "PRE_DEC"); }
    | expr INC   %prec UNARY    { $$ = createUnaryExpNode($1, "POST_INC"); }
    | expr DEC   %prec UNARY    { $$ = createUnaryExpNode($1, "POST_DEC"); }
    | ID                        { $$ = createTermExpNode(createIdRefNode($1)); } 
    | INT_LITERAL               { $$ = createTermExpNode(createIntLiteralNode($1)); }
    | CHAR_LITERAL              { $$ = createTermExpNode(createCharLiteralNode($1)); }
    | func_call                 { $$ = $1; }
    | '(' expr ')'              { $$ = $2; }
    ;



%%


void yyerror(const char* s) {
    fprintf(stderr, "Error: %s at line %d, character %d\n", s, cur_line, cur_char);
    exit(0);
}

int main(int argc, char *argv[]){
    int exportAST_flag = 0;  // Flag to determine whether to export AST or not
    int printAST_flag = 0;
    int printSymTable_flag = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--export-ast") == 0) {
            exportAST_flag = 1;
        }
        if (strcmp(argv[i], "--print-ast") == 0) {
            printAST_flag = 1;
        }
        if (strcmp(argv[i], "--print-sym-table") == 0) {
            printSymTable_flag = 1;
        }
    }

  
    symTable = createSymbolTable("global", NULL, 100);
    currentScope = symTable; // Initial current scope
    
    yyparse();

    SemanticStatus sem_stat = performSemanticAnalysis(root, symTable);

    if(sem_stat == SEMANTIC_SUCCESS)
        printf("\nPARSING SUCCESS\n");
    
    printf("\n\n");

    if(printAST_flag){
        printAST(root, 0, false);
        printf("\n\n");
    } 

    
    if(printSymTable_flag){
        printf("\n\n");
        printSymbolTable(symTable);
    }  

    // If --export-ast option is provided, export the AST as JSON
    if (exportAST_flag) {
        exportASTAsJSON(folderPathForAST_Vis, root);
    }


    freeAST(root);
    freeSymbolTable(symTable);
}



// Wrapper function to create a program node with a list of statements
ASTNode* createProgramNode(ASTNode* stmt_list) {
    // Create a node for the root of the program
    ASTNode* programNode = createASTNode(NODE_PROGRAM);

    programNode->program_data.stmt_list = stmt_list;
    programNode->program_data.scope = symTable;
    
    return programNode;
}

ASTNode* createStmtListNode(ASTNode* stmt_list, ASTNode* stmt) {
    ASTNode* node = createASTNode(NODE_STMT_LIST);
    node->stmt_list_data.stmt_list = stmt_list; // Pointer to the next statement or NULL
    node->stmt_list_data.stmt = stmt;
    return node;
}

ASTNode* createBlockStmtNode(ASTNode* stmt_list){
    ASTNode* node = createASTNode(NODE_BLOCK_STMT);
    node->block_stmt_data.stmt_list = stmt_list;
    
    return node;
}



ASTNode* createReturnNode(ASTNode* return_value){
    ASTNode* node = createASTNode(NODE_RETURN);
    node->return_data.return_value = return_value;

    return node;
}


// EXPRESSIONS

ASTNode* createBinaryExpNode(ASTNode* left, ASTNode* right, const char* op) {
    ASTNode* node = createASTNode(NODE_EXPR_BINARY);
    node->expr_data.left = left;
    node->expr_data.right = right;
    node->expr_data.op = strdup(op);  // Store the operation
    return node;
}

ASTNode* createUnaryExpNode(ASTNode* left, const char* op){
    ASTNode* node = createASTNode(NODE_EXPR_UNARY);
    node->expr_data.left = left;
    node->expr_data.right = NULL;
    node->expr_data.op = strdup(op);  // Store the operation
    return node;
}

ASTNode* createTermExpNode(ASTNode* term){
    ASTNode* node = createASTNode(NODE_EXPR_TERM);
    node->expr_data.left = term;
    node->expr_data.right = NULL;
    node->expr_data.op = NULL;
    return node;
}



// IDENTIFIERS

ASTNode* createIdentifierNode(const char* id, char* type) {
    ASTNode* node = createASTNode(NODE_ID);

    symbol* sym = createSymbol(id, type, currentScope, -1, 0, cur_line, cur_char);
    node->id_data.sym = sym;
    addSymbol(currentScope, sym);
    return node;
}

ASTNode* createFucnIdNode(const char* id, ASTNode* type_spec){
    ASTNode* node = createASTNode(NODE_ID);
    char* type = (char*)type_spec->type_data.type;

    symbol* sym = createSymbol(id, type, currentScope, -1, 1, cur_line, cur_char);
    node->id_data.sym = sym;
    addSymbol(currentScope, sym);
    return node;  
}

ASTNode* createIdRefNode(const char* id){
    ASTNode* node = createASTNode(NODE_ID_REF);
    node->id_ref_data.name = id;
    node->id_ref_data.ref = NULL;
    node->id_ref_data.scope = currentScope;
    node->id_ref_data.line_no = cur_line;
    node->id_ref_data.char_no = cur_char;
    return node;
}



// LITERALS

ASTNode* createIntLiteralNode(int i){
    ASTNode* node = createASTNode(NODE_INT_LITERAL);
    node->literal_data.value.int_value = i;
    return node;
}
ASTNode* createCharLiteralNode(char c){
    ASTNode* node = createASTNode(NODE_CHAR_LITERAL);
    node->literal_data.value.char_value = c;
    return node;
}
ASTNode* createStrLiteralNode(const char* s){
    ASTNode* node = createASTNode(NODE_STR_LITERAL);
    node->literal_data.value.str_value = s;
    return node;
}



// DECLARATIONS, VARIABLES and ASSIGNMENTS

ASTNode* createDeclNode(ASTNode* type_spec, ASTNode* var_list){
    ASTNode* node = createASTNode(NODE_DECL);
    node->decl_data.type_spec = type_spec;
    node->decl_data.var_list = var_list;

    return node;
}

ASTNode* createTypeNode(char* type){
    ASTNode* node = createASTNode(NODE_TYPE_SPEC);
    node->type_data.type = type;
    return node;
}

ASTNode* createVarListNode(ASTNode* var_list, ASTNode* var){
    ASTNode* node = createASTNode(NODE_VAR_LIST);
    node->var_list_data.var_list = var_list;
    node->var_list_data.var = var;

    return node;
}

ASTNode* createVarNode(const char* id) {
    ASTNode* node = createASTNode(NODE_VAR);
    node->var_data.id = createIdentifierNode(id, NULL);  // Simple variable
    node->var_data.value = NULL;
    return node;
}

ASTNode* createVarAssgnNode(const char* id, ASTNode* value){
   ASTNode* node = createASTNode(NODE_VAR);
   node->var_data.id = createIdentifierNode(id, NULL);
   node->var_data.value = value;
   return node;
}

ASTNode* createAssgnNode(const char* id, ASTNode* value){
    ASTNode* node = createASTNode(NODE_ASSGN);
    node->assgn_data.left = createIdRefNode(id);
    node->assgn_data.right = value;

    return node;
}

void setVarListType(ASTNode* typeNode, ASTNode* var_list) {
    if (var_list == NULL) return;

    const char* type = typeNode->type_data.type;

    if (var_list->type == NODE_VAR_LIST) {
        // If it's a var_list, recursively update both parts (var_list and individual var)
        setVarListType(typeNode, var_list->var_list_data.var_list);
        setVarListType(typeNode, var_list->var_list_data.var);
    } else if (var_list->type == NODE_VAR) {
        // For an individual variable, check if it's an assignment or just a declaration
        if (var_list->var_data.id != NULL && var_list->var_data.id->id_data.sym != NULL) {
            symbol* sym = var_list->var_data.id->id_data.sym;
            if (sym->type == NULL) {
                sym->type = strdup(type); // Assign the type to the variable's symbol
            }
        }

    }
}



// IF ELSE 

ASTNode* createIfNode(ASTNode* cond, ASTNode* if_branch) {
    ASTNode* node = createASTNode(NODE_IF);

    // Implicit creation of cond & branch nodes

    ASTNode* node_cond = createASTNode(NODE_IF_COND);
    node_cond->if_cond_data.cond = cond;
    node->if_else_data.condition = node_cond;

    ASTNode* node_if_branch = createASTNode(NODE_IF_BRANCH);
    node_if_branch->if_else_branch.branch = if_branch;
    node->if_else_data.if_branch = node_if_branch;

    node->if_else_data.else_branch = NULL;
    return node;
}

ASTNode* createIfElseNode(ASTNode* cond, ASTNode* if_branch, ASTNode* else_branch){
    ASTNode* node = createASTNode(NODE_IF_ELSE);

    // Implicit creation of cond & branch nodes

    ASTNode* node_cond = createASTNode(NODE_IF_COND);
    node_cond->if_cond_data.cond = cond;
    node->if_else_data.condition = node_cond;

    ASTNode* node_if_branch = createASTNode(NODE_IF_BRANCH);
    node_if_branch->if_else_branch.branch = if_branch;
    node->if_else_data.if_branch = node_if_branch;
   
    ASTNode* node_else_branch = createASTNode(NODE_ELSE_BRANCH);
    node_else_branch->if_else_branch.branch = else_branch;
    node->if_else_data.else_branch = node_else_branch;
    
    return node;
}

ASTNode* createWhileNode(ASTNode* cond, ASTNode* body){
    ASTNode* node = createASTNode(NODE_WHILE);

    // Implict creation of cond and branch nodes
    ASTNode* node_while_cond = createASTNode(NODE_WHILE_COND);
    node_while_cond->while_cond_data.cond = cond;

    ASTNode* node_while_body = createASTNode(NODE_WHILE_BODY);
    node_while_body->while_body_data.body = body;

    node->while_data.condition = node_while_cond;
    node->while_data.while_body = node_while_body;

    return node;

}

ASTNode* createForNode(ASTNode* init, ASTNode* cond, ASTNode* updation, ASTNode* body){
    ASTNode* node = createASTNode(NODE_FOR);
    

    // Implicit creation of init, cond and updation nodes;
    ASTNode* node_for_init = createASTNode(NODE_FOR_INIT);
    node_for_init->for_init_data.init = init;

    ASTNode* node_for_cond = createASTNode(NODE_FOR_COND); 
    node_for_cond->for_cond_data.cond = cond;

    ASTNode* node_for_upd = createASTNode(NODE_FOR_UPDATION);
    node_for_upd->for_updation_data.updation = updation;

    ASTNode* node_for_body = createASTNode(NODE_FOR_BODY);
    node_for_body->for_body_data.body = body;

    node->for_data.init = node_for_init;
    node->for_data.condition = node_for_cond;
    node->for_data.updation = node_for_upd;
    node->for_data.body = node_for_body;

    return node;
}


ASTNode* createCommaExprList(ASTNode* expr_list, ASTNode* expr_list_item){
    ASTNode* node = createASTNode(NODE_EXPR_COMMA_LIST);

    node->expr_comma_list_data.expr_comma_list = expr_list;
    node->expr_comma_list_data.expr_comma_list_item = expr_list_item;

    return node;
}

int countParams(ASTNode* params) {
    if (params == NULL) return 0;

    // Check if the node is a parameter list node
    if (params->type == NODE_PARAM_LIST) {
        return countParams(params->param_list_data.param_list) + countParams(params->param_list_data.param);
    }

    return 1;
}


ASTNode* createFuncDeclNode(ASTNode* type_spec, ASTNode* id, ASTNode* params, ASTNode* body){
    ASTNode* node = createASTNode(NODE_FUNC_DECL);

    node->func_decl_data.id = id;
    node->func_decl_data.params = params;
    node->func_decl_data.param_count = countParams(params);
    /* printf("Params count: %d\n", node->func_decl_data.param_count); */

    // Implicit creation of Function body node
    ASTNode* body_node = createASTNode(NODE_FUNC_BODY);
    body_node->func_body_data.body = body;

    node->func_decl_data.body = body_node;


    return node;
}

ASTNode* createParamsListNode(ASTNode* params_list, ASTNode* param){
    ASTNode* node = createASTNode(NODE_PARAM_LIST);

    node->param_list_data.param_list = params_list;
    node->param_list_data.param = param;

    return node;
}

ASTNode* createParamNode(ASTNode* type_spec, const char* id){
    ASTNode* node = createASTNode(NODE_PARAM);
    char* type = (char*)type_spec->type_data.type;
    node->param_data.type_spec = type_spec;
    node->param_data.id = createIdentifierNode(id, type);
    return node;
}

int countArgs(ASTNode* arg_list) {
    if (arg_list == NULL) return 0;

    // Check if the node is a parameter list node
    if (arg_list->type == NODE_ARG_LIST) {
        return countParams(arg_list->arg_list_data.arg_list) + countParams(arg_list->arg_list_data.arg);
    }

    return 1;
}

ASTNode* createFuncCallNode(const char* id, ASTNode* arg_list){
    ASTNode* node = createASTNode(NODE_FUNC_CALL);


    node->func_call_data.id = createIdRefNode(id);
    node->func_call_data.arg_list = arg_list;
    node->func_call_data.arg_count = countArgs(arg_list);

    return node;

}

ASTNode* createArgListNode(ASTNode* arg_list, ASTNode* arg){
    ASTNode* node = createASTNode(NODE_ARG_LIST);

    node->arg_list_data.arg_list = arg_list;
    node->arg_list_data.arg = createArgNode(arg);

    return node;
}

ASTNode* createArgNode(ASTNode* arg){
    ASTNode* node = createASTNode(NODE_ARG);

    node->arg_data.arg = arg;
    return node;
}