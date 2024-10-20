%{
    #include<stdio.h>
    #include<stdlib.h>
    #include<string.h>

    // Global variables to track line and character numbers
    int current_line = 1;
    int current_char = 0;

    void yyerror(const char* s);
    int yylex(void);    
%}

%union {
    int ival;
    char* strval;
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

%type <ival> expr

%%

// Program structure
program: 
    stmt_list
    ;

// List of statements
stmt_list:
    stmt_list stmt
    |
    ;

// Different types of statements
stmt:
    decl_stmt
    | assgn_stmt
    | expr_stmt
    | cond_stmt
    | block_stmt
    | loop_stmt
    | ret_stmt
    | func_decl
    | func_call_stmt
    | ';'
    ;

// Return statement
ret_stmt:
    RETURN ret_val ';'
    ;
ret_val:
    expr | ;

// Function call statement
func_call_stmt:
    func_call ';'
    ;

// Assignment statement
assgn_stmt:
    assgn_expr ';'
    ;
assgn_expr:
    ID ASSIGN expr 
    | ID ASSIGN STR_LITERAL 
    ;

// Block statement
block_stmt:
    '{' stmt_list '}';

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
    decl
    | expr_list
    | 
    ;

// Expression list for for loop
expr_list:
    expr_list ',' expr_list_item
    | expr_list_item
    ;
expr_list_item:
    assgn_expr | expr ;

for_expr:
    expr | ;



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
    INT | CHAR | FLOAT | STRING;

// Variable list for declarations
var_list:
    var_list ',' var 
    | var ;

// Variable definitions
var :
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
    | expr
    | 
    ;

// Parameter list for function declarations
params:
    param ',' params
    | param 
    | 
    ;

// Parameter definition
param:
    type_spec ID 
    ;

// Expression statement
expr_stmt:
    expr ';'
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
    | MINUS expr %prec UNARY    { $$ = -$2; } /* unary minus */
    | ID                        { $$ = 0; /* placeholder */ }
    | func_call                 { $$ = 0;}    
    | INC expr   %prec UNARY    { $$ = 0;}  
    | DEC expr   %prec UNARY    { $$ = 0;}  
    | expr INC   %prec UNARY    { $$ = 0;}  
    | expr DEC   %prec UNARY    { $$ = 0;}  
    | INT_LITERAL               { $$ = $1; }
    | CHAR_LITERAL              { $$ = $1; }
    | '(' expr ')'              { $$ = $2; }
    ;
%%


void yyerror(const char* s) {
    fprintf(stderr, "Error: %s\n", s);
    exit(0);
}

int main(){
    yyparse();

    printf("PARSING SUCCESS\n");
}
