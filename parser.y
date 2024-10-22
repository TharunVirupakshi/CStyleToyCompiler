%{
#include "symTable.h"
#include "ast.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
// Declare extern to access the variables defined in lexer
extern int cur_line;
extern int cur_char;

void yyerror(const char* s);
int yylex(void);

SymbolTable* symTable;

ASTNode* root;
ASTNode* createProgramNode(ASTNode* stmt_list);
ASTNode* createStmtListNode(ASTNode* stmtList, ASTNode* stmt);
ASTNode* createBlockStmtNode(ASTNode* stmt_list);
ASTNode* createReturnNode(ASTNode* return_value);
ASTNode* createBinaryExpNode(ASTNode* left, ASTNode* right, const char* op);
ASTNode* createUnaryExpNode(ASTNode* left, const char* op);
ASTNode* createTermExpNode(ASTNode* term);
ASTNode* createIdentifierNode(const char* id);
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
%}



%union {
    int ival;
    const char* strval;     
    ASTNode* node;
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
%type <node> var_list var param params arg_list expr_stmt expr_list_item
%%

// Program structure
program: 
    stmt_list  { $$ = createProgramNode($1); root = $$; }
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
    ID ASSIGN expr 
    | ID ASSIGN STR_LITERAL 
    ;

// Block statement
block_stmt:
    '{' stmt_list '}'   { $$ = createBlockStmtNode($2); } 
    ;

// Conditional statement
cond_stmt:
    IF '(' expr ')' stmt 
    | IF '(' expr ')' stmt ELSE stmt 
    ;

// Loop statements
loop_stmt:
    WHILE '(' expr ')' stmt 
    | FOR '(' for_init ';' for_expr ';' for_expr ')' stmt 
    ; 

for_init:
    decl            { $$ = $1; }
    | expr_list     { $$ = $1; }
    |               
    ;

// Expression list for for loop
expr_list:
    expr_list ',' expr_list_item  
    | expr_list_item              { $$ = $1; }
    ;
expr_list_item:
    assgn_expr | expr ;

for_expr:
    expr            { $$ = $1; }
    |              
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
    type_spec ID '(' params ')' stmt
    | VOID ID '(' params ')' stmt 
    ;

// Function calls
func_call:
    ID '(' arg_list ')' 
    ;

// Argument list for function calls
arg_list:
    expr ',' arg_list 
    | expr { 
        $$ = $1; 
    }
    | { 
        $$ = NULL; 
    }
    ;

// Parameter list for function declarations
params:
    param ',' params 
    | param { 
        $$ = $1; 
    }
    | { 
        $$ = NULL; 
    }
    ;

// Parameter definition
param:
    type_spec ID 
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

int main(){
    symTable = createSymbolTable(100);
    
    yyparse();

    printf("\nPARSING SUCCESS\n");
    printf("\n\n");
    printAST(root, 0, false);
    printf("\n\n");
    printSymbolTable(symTable);
    freeAST(root);
    freeSymbolTable(symTable);
}



// Wrapper function to create a program node with a list of statements
ASTNode* createProgramNode(ASTNode* stmt_list) {
    // Create a node for the root of the program
    ASTNode* programNode = createASTNode(NODE_PROGRAM);
    programNode->program_data.stmt_list = stmt_list;
    // Attach the statement list as the child
    /* programNode->children[0] = stmt_list; */
    
    return programNode;
}

ASTNode* createStmtListNode(ASTNode* stmt, ASTNode* stmt_list) {
    ASTNode* node = createASTNode(NODE_STMT_LIST);
    node->stmt_list_data.stmt = stmt;
    node->stmt_list_data.stmt_list = stmt_list; // Pointer to the next statement or NULL
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

ASTNode* createIdentifierNode(const char* id) {
    ASTNode* node = createASTNode(NODE_ID);
    symbol* sym = createSymbol(id, NULL, 0, -1, 0, node);
    node->id_data.sym = sym;
    addSymbol(symTable, sym);
    return node;
}

ASTNode* createIdRefNode(const char* id){
    ASTNode* node = createASTNode(NODE_ID_REF);
    node->id_ref_data.name = id;
    node->id_ref_data.ref = NULL;

    return node;
}

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
    node->var_data.id = createIdentifierNode(id);  // Simple variable
    node->var_data.value = NULL;
    return node;
}

ASTNode* createVarAssgnNode(const char* id, ASTNode* value){
   ASTNode* node = createASTNode(NODE_VAR);
   node->var_data.id = createIdentifierNode(id);
   node->var_data.value = value;
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

