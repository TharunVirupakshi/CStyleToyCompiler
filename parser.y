%{
#include "ast.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
// Declare extern to access the variables defined in lexer
extern int cur_line;
extern int cur_char;

void yyerror(const char* s);
int yylex(void);




ASTNode* root;
ASTNode* createProgramNode(ASTNode* stmt_list);
%}



%union {
    int ival;
    char* strval;     
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
    stmt_list  { $$ = createProgramNode($1); }
    ;

// List of statements
stmt_list:
    stmt_list stmt  
    |               
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
    | ';'               
    ;

// Return statement
ret_stmt:
    RETURN expr ';'     
    | RETURN ';'        
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
    '{' stmt_list '}' 
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
    decl ';'        
    ;

// Declaration
decl:
    type_spec var_list  
    ;

// Type specifications
type_spec:
    INT                 
    | CHAR              
    | FLOAT             
    | STRING            
    ;

// Variable list for declarations
var_list:
    var_list ',' var    
    | var               { $$ = $1; }
    ;             

// Variable definitions
var:
    ID                      
    | ID ASSIGN expr        
    | ID ASSIGN STR_LITERAL 
    | ID '[' INT_LITERAL ']' 
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
    expr PLUS expr              
    | expr MINUS expr           
    | expr MULT expr            
    | expr DIV expr             
    | expr EQ expr              
    | expr NEQ expr             
    | expr LT expr              
    | expr GT expr              
    | expr LEQ expr             
    | expr GEQ expr             
    | expr AND expr             
    | expr OR expr              
    | NOT expr                  { $$ = $2; }
    | MINUS expr %prec UNARY    { $$ = $2; }
    | ID                        { $$ = 0; } 
    | func_call                 { $$ = $1; }
    | INC expr   %prec UNARY    
    | DEC expr   %prec UNARY    
    | expr INC   %prec UNARY    
    | expr DEC   %prec UNARY    
    | INT_LITERAL               
    | CHAR_LITERAL              
    | '(' expr ')'              { $$ = $2; }
    ;

%%


void yyerror(const char* s) {
    fprintf(stderr, "Error: %s at line %d, character %d\n", s, cur_line, cur_char);
    exit(0);
}

int main(){
    yyparse();

    printf("PARSING SUCCESS\n");
}

// Wrapper function to create a program node with a list of statements
ASTNode* createProgramNode(ASTNode* stmt_list) {
    // Create a node for the root of the program
    ASTNode* programNode = createASTNode(NODE_PROGRAM, 1);
    
    // Attach the statement list as the child
    programNode->children[0] = stmt_list;
    
    return programNode;
}

ASTNode* createStmtListNode(ASTNode* stmtList, ASTNode* stmt) {
    if (stmtList == NULL) {
        // First statement in the list, create a new stmt_list node
        ASTNode* stmtListNode = createASTNode(NODE_STMT_LIST, 1);
        stmtListNode->children[0] = stmt;
        stmtListNode->child_count = 1;
        return stmtListNode;
    } else {
        // Add a new statement to the existing stmt_list
        int newCount = stmtList->child_count + 1;
        stmtList->children = realloc(stmtList->children, newCount * sizeof(ASTNode*));
        stmtList->children[stmtList->child_count] = stmt;
        stmtList->child_count = newCount;
        return stmtList;
    }
}