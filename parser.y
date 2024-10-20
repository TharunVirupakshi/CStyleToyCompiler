%{
    #include<stdio.h>
    #include<stdlib.h>
    #include<string.h>

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

%right ASSIGN
%left EQ NEQ
%left LT GT LEQ GEQ
%left PLUS MINUS
%left MULT DIV
%nonassoc UNARY



%type <ival> expr


%%
program: 
    stmt_list
    ;

stmt_list:
    stmt_list stmt
    |
    ;

stmt:
    decl_stmt
    | assgn_stmt
    | expr_stmt
    | cond_stmt
    | block_stmt
    | loop_stmt
    | ';'
    ;

assgn_stmt:
    assgn_expr ';'
    ;

assgn_expr:
    ID ASSIGN expr 
    | ID ASSIGN STR_LITERAL 
    ;

block_stmt:
    '{' stmt_list '}';

cond_stmt:
    IF '(' expr ')' stmt
    | IF '(' expr ')' stmt ELSE stmt
    ;

loop_stmt:
    WHILE '(' expr ')' stmt
    | FOR '(' for_init ';' for_expr ';' for_expr ')' stmt
    ; 

for_init:
    decl
    | expr_list
    | 
    ;

expr_list:
    expr_list ',' expr_list_item
    | expr_list_item
    ;
expr_list_item:
    assgn_expr | expr ;

for_expr:
    expr | ;

decl_stmt:
    decl ';'
    ;

decl:
    type_spec var_list
    ;

type_spec:
    INT | CHAR | FLOAT | STRING;

var_list:
    var_list ',' var 
    | var ;

var :
    ID
    | ID ASSIGN expr
    | ID ASSIGN STR_LITERAL
    | ID '[' INT_LITERAL ']'
    ;

expr_stmt:
    expr ';'
    ;

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
    | MINUS expr %prec UNARY    { $$ = -$2; } /* unary minus */
    | ID                        { $$ = 0; /* placeholder */ }
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