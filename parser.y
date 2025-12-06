%{
#include "symTable.h"
#include "ast.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>
#include "semantic.h"
#include "icg.h"
#include "logger.h"

#define GLOBAL "global"
#define FUNCTION "function"
#define BLOCK "block"
#define LOGGER_NAME "PARSER"  

bool isSemanticError = false;
bool isParserDebuggerOn = false;

// Declare extern to access the variables defined in lexer
extern int cur_line;
extern int cur_char;

void yyerror(const char* s);
int yylex(void);

// Global Scope
SymbolTable* symTable;
// Current Scope
SymbolTable* currentScope;

// List of break/continue stmts
BrkCntStmtsList* brkCntList = NULL;
BrkCntStmtsList* brkCntListHEAD = NULL;

const char *folderPathForAST_Vis = "AST_Vis"; 
int scopeDepth = 0;
int func_id = 1;

void log_rule(const char* rule, int ruleId, int subRuleId) {
    Step s;
    s.type = PARSE_REDUCE_RULE;
    s.reduceRule.rule = rule;
    s.reduceRule.subRuleId = subRuleId;
    s.reduceRule.ruleId = ruleId;
    log_step(s);
}

void log_assgn_sym_type(const char* sym_name, int scope_id, const char* type) {
    Step s;
    s.type = PARSE_ASSGN_SYM_TYPE;
    s.AssignSymType.name = sym_name;
    s.AssignSymType.scope_id = scope_id;
    s.AssignSymType.type = (char *)type;
    log_step(s);
}

void printLog(const char *fmt, ...) {
    printf("[%s] ", LOGGER_NAME);

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);   // prints the formatted string
    va_end(args);

    printf("\n");
}

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
ASTNode* createBreakNode();
ASTNode* createContinueNode();
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
%token IF ELSE WHILE FOR RETURN BREAK CONTINUE INT FLOAT CHAR VOID STRING
%token PLUS MINUS MULT DIV INC DEC
%token EQ NEQ LEQ GEQ LT GT ASSIGN
%token AND OR NOT

%right ASSIGN
%left OR
%left AND
%left EQ NEQ
%left LT GT LEQ GEQ
%left PLUS MINUS
%left MULT DIV
%nonassoc UNARY

%type <node> program type_spec expr expr_list stmt block_stmt decl_stmt decl cond_stmt loop_stmt 
%type <node> assgn_expr func_call func_call_stmt func_decl stmt_list ret_stmt break_stmt continue_stmt assgn_stmt for_init for_expr
%type <node> var_list var param params arg_list expr_stmt expr_list_item body block_stmt_without_scope 
%type <func_header_data> func_header
%type <cond_footer_data> else_part

%%

// Program structure
program: 
    stmt_list  { 
        log_rule("program → stmt_list", 1, 1);
        $$ = createProgramNode($1); 
        root = $$; 
    }
    ;

// List of statements
stmt_list:
      stmt_list stmt { 
          log_rule("stmt_list → stmt_list stmt", 2, 1);
          $$ = createStmtListNode($1, $2); 
      } 
    | { 
          log_rule("stmt_list → ε", 2, 2);
          $$ = NULL;
      }  
;
            
// Different types of statements
stmt:
      decl_stmt { 
          log_rule("stmt → decl_stmt", 3, 1); 
          $$ = $1; 
      }
    | assgn_stmt { 
          log_rule("stmt → assgn_stmt", 3, 2); 
          $$ = $1; 
      }
    | expr_stmt { 
          log_rule("stmt → expr_stmt", 3, 3); 
          $$ = $1; 
      }
    | cond_stmt { 
          log_rule("stmt → cond_stmt", 3, 4); 
          $$ = $1; 
      }         
    | block_stmt { 
          log_rule("stmt → block_stmt", 3, 5); 
          $$ = $1; 
      }    
    | loop_stmt { 
          log_rule("stmt → loop_stmt", 3, 6); 
          $$ = $1; 
      }
    | ret_stmt { 
          log_rule("stmt → ret_stmt", 3, 7); 
          $$ = $1; 
      }
    | func_decl { 
          log_rule("stmt → func_decl", 3, 8); 
          $$ = $1; 
      }
    | func_call_stmt { 
          log_rule("stmt → func_call_stmt", 3, 9); 
          $$ = $1; 
      }
    | break_stmt { 
          log_rule("stmt → break_stmt", 3, 10); 
          $$ = $1; 
      }
    | continue_stmt { 
          log_rule("stmt → continue_stmt", 3, 11); 
          $$ = $1; 
      }
    | ';' { 
          log_rule("stmt → ';'", 3, 12);
          $$ = NULL; 
      }
;



// Return statement
ret_stmt:
      RETURN expr ';' { 
          log_rule("ret_stmt → RETURN expr ;", 4, 1); 
          $$ = createReturnNode($2); 
      }   
    | RETURN ';' { 
          log_rule("ret_stmt → RETURN ;", 4, 2); 
          $$ = createReturnNode(NULL);
      }
;



// Function call statement
func_call_stmt:
    func_call ';' { 
        log_rule("func_call_stmt → func_call ;", 5, 1);
        $$ = $1; 
    }
    ;

// Assignment statement
assgn_stmt:
    assgn_expr ';' {
        log_rule("assgn_stmt → assgn_expr ;", 6, 1);
        $$ = $1; 
    }
    ;

assgn_expr:
    ID ASSIGN expr {
        log_rule("assgn_expr → ID ASSIGN expr", 6, 2);
        $$ = createAssgnNode($1, $3); 
    } 
    ;

// Block statement
block_stmt:
    '{' { 
        log_rule("block_stmt → { stmt_list }", 7, 1); 
        currentScope = enterScope("block", currentScope);
    } 
    stmt_list 
    '}' { 
        $$ = createBlockStmtNode($3); 
        currentScope = exitScope(currentScope); 
    } 
    ;

block_stmt_without_scope: 
    '{' stmt_list '}' { 
        log_rule("block_stmt_without_scope → { stmt_list }", 8, 1);
        $$ = createBlockStmtNode($2); 
    }
    ;

/*
    Generic body where the first block stmt will not have its own scope
*/
body:
    block_stmt_without_scope { 
        log_rule("body → block_stmt_without_scope", 9, 1);
        $$ = $1; 
    }
    | decl_stmt { 
        log_rule("body → decl_stmt", 9, 2);
        $$ = $1; 
    }
    | assgn_stmt { 
        log_rule("body → assgn_stmt", 9, 3);
        $$ = $1; 
    }
    | expr_stmt { 
        log_rule("body → expr_stmt", 9, 4);
        $$ = $1; 
    }
    | cond_stmt { 
        log_rule("body → cond_stmt", 9, 5);
        $$ = $1; 
    }         
    | loop_stmt { 
        log_rule("body → loop_stmt", 9, 6);
        $$ = $1; 
    }
    | ret_stmt { 
        log_rule("body → ret_stmt", 9, 7);
        $$ = $1; 
    }
    | func_decl { 
        log_rule("body → func_decl", 9, 8);
        $$ = $1; 
    }
    | func_call_stmt { 
        log_rule("body → func_call_stmt", 9, 9);
        $$ = $1; 
    }
    | break_stmt { 
        log_rule("body → break_stmt", 9, 10);
        $$ = $1; 
    }
    | continue_stmt { 
        log_rule("body → continue_stmt", 9, 11);
        $$ = $1; 
    }
    | ';' { 
        log_rule("body → ;", 9, 12);
        $$ = NULL; 
    }
    ;

break_stmt:
    BREAK ';' { 
        log_rule("break_stmt → BREAK ;", 10, 1);
        $$ = createBreakNode(); 
    }
    ;


continue_stmt:
    CONTINUE ';' { 
        log_rule("continue_stmt → CONTINUE ;", 11, 1);
        $$ = createContinueNode(); 
    }
    ;

// Conditional statement
cond_stmt:
    IF '(' expr ')' {
        log_rule("cond_stmt → IF '(' expr ')' body else_part", 12, 1);
        currentScope = enterScope("block (if)", currentScope);
    } body {
        currentScope = exitScope(currentScope);
    } else_part {
        $$ = createIfElseNode($3, $6, $8.else_body); 
    } 

else_part:
    ELSE {
        log_rule("else_part → body", 13, 1); 
        currentScope = enterScope("block (else)", currentScope);
    } body { 
        $$.else_body = $3; 
        currentScope = exitScope(currentScope); 
    }
    | { log_rule("else_part → ε", 13, 2); $$.else_body = NULL; }
    ;



// Loop statements
loop_stmt:
    WHILE '(' expr ')' {
        log_rule("loop_stmt → WHILE '(' expr ')' body", 14, 1);
        currentScope = enterScope("block (while)", currentScope);
    } body { 
        $$ = createWhileNode($3, $6); 
        currentScope = exitScope(currentScope); 
    } 
    | FOR '(' {
        log_rule("loop_stmt → FOR '('", 14, 2);
        currentScope = enterScope("block (for)", currentScope);} for_init ';' for_expr ';' for_expr ')' body {
        $$ = createForNode($4, $6, $8, $10);
        currentScope = exitScope(currentScope); 
    }
    ; 

for_init:
    decl            { log_rule("for_init → decl", 15, 1); $$ = $1; }
    | expr_list     { log_rule("for_init → expr_list", 15, 2); $$ = $1; }
    |               { log_rule("for_init → ε", 15, 3); $$ = NULL; }
    ;

// Expression list for for loop
expr_list:
    expr_list ',' expr_list_item  { log_rule("expr_list → expr_list ',' expr_list_item", 16, 1); $$ = createCommaExprList($1, $3); }
    | expr_list_item              { log_rule("expr_list → expr_list_item", 16, 2); $$ = createCommaExprList(NULL, $1); }
    ;
expr_list_item:
    assgn_expr      { log_rule("expr_list_item → assgn_expr", 17, 1); $$ = $1; }                   
    | expr          { log_rule("expr_list_item → expr", 17, 2); $$ = $1; }
    ;

for_expr:
    expr_list       { log_rule("for_expr → expr_list", 18, 1); $$ = $1; }
    |               { log_rule("for_expr → ε", 18, 2); $$ = NULL; }             
    ;



// Declaration statement
decl_stmt:
    decl ';'        { log_rule("decl_stmt → decl ';'", 19, 1); $$ = $1; } 
    ;

// Declaration
decl:
    type_spec var_list  {
        log_rule("decl → type_spec var_list", 20, 1); 
        setVarListType($1, $2); 
        $$ = createDeclNode($1, $2); 
    }
    ;

// Type specifications
type_spec:
    INT       { log_rule("type_spec → INT", 21, 1); $$ = createTypeNode("int"); }               
    | CHAR    { log_rule("type_spec → CHAR", 21, 2); $$ = createTypeNode("char"); }     
    | FLOAT   { log_rule("type_spec → FLOAT", 21, 3); $$ = createTypeNode("float"); }          
    | STRING  { log_rule("type_spec → STRING", 21, 4); $$ = createTypeNode("string"); }           
    ;

// Variable list for declarations
var_list:
    var_list ',' var    { log_rule("var_list → var_list ',' var", 22, 1); $$ = createVarListNode($1, $3); }
    | var               { log_rule("var_list → var", 22, 2); $$ = createVarListNode(NULL, $1); }
    ;             

// Variable definitions
var:
    ID                          { log_rule("var → ID", 23, 1); $$ = createVarNode($1); } 
    | ID ASSIGN expr            { log_rule("var → ID ASSIGN expr", 23, 2); $$ = createVarAssgnNode($1, $3); }
    ;

// Function declarations
func_decl:
    func_header params ')' body { 
        log_rule("func_decl → func_header params ')' body", 24, 1);
        $$ = createFuncDeclNode($1.type, $1.id, $2, $4);
        currentScope = exitScope(currentScope);
    }
    ;

func_header:
    type_spec ID '(' {
        log_rule("func_header → type_spec ID '('", 25, 1);
        $$.type = $1;
        $$.id = createFucnIdNode($2, $1);
        currentScope = enterScope((char*)$2, currentScope);
    }
    | VOID ID '(' {
        log_rule("func_header → VOID ID '('", 25, 2);
        $$.type = createTypeNode("void");
        $$.id = createFucnIdNode($2, $$.type);
        currentScope = enterScope((char*)$2, currentScope); 
    }
    ;

// Function calls
func_call:
    ID '(' arg_list ')' { 
        log_rule("func_call → ID '(' arg_list ')'", 26, 1); 
        $$ = createFuncCallNode($1, $3); 
    }
    ;

// Argument list for function calls
arg_list:
    arg_list ',' expr { 
        log_rule("arg_list → arg_list ',' expr", 27, 1); 
        $$ = createArgListNode($1, $3); 
    } 
    | expr { 
        log_rule("arg_list → expr", 27, 2); 
        $$ = createArgListNode(NULL, $1); 
    }
    | { 
        log_rule("arg_list → ε", 27, 3); 
        $$ = NULL; 
    }
    ;

// Parameter list for function declarations
params:
    params ',' param { 
        log_rule("params → params ',' param", 28, 1); 
        $$ = createParamsListNode($1, $3); 
    }
    | param { 
        log_rule("params → param", 28, 2); 
        $$ = createParamsListNode(NULL, $1); 
    }
    | { 
        log_rule("params → ε", 28, 3); 
        $$ = NULL; 
    }
    ;

// Parameter definition
param:
    type_spec ID { 
        log_rule("param → type_spec ID", 29, 1); 
        $$ = createParamNode($1, $2); 
    } 
    ;


// Expression statement
expr_stmt:
    expr ';' { 
        log_rule("expr_stmt → expr ;", 30, 1); 
        $$ = $1; 
    }
    ;

// Expressions
expr:
    expr PLUS expr          { log_rule("expr → expr PLUS expr", 31, 1); $$ = createBinaryExpNode($1, $3, "+"); }              
    | expr MINUS expr       { log_rule("expr → expr MINUS expr", 31, 2); $$ = createBinaryExpNode($1, $3, "-"); }
    | expr MULT expr        { log_rule("expr → expr MULT expr", 31, 3); $$ = createBinaryExpNode($1, $3, "*"); }
    | expr DIV expr         { log_rule("expr → expr DIV expr", 31, 4); $$ = createBinaryExpNode($1, $3, "/"); }
    | expr EQ expr          { log_rule("expr → expr EQ expr", 31, 5); $$ = createBinaryExpNode($1, $3, "=="); }
    | expr NEQ expr         { log_rule("expr → expr NEQ expr", 31, 6); $$ = createBinaryExpNode($1, $3, "!="); }
    | expr LT expr          { log_rule("expr → expr LT expr", 31, 7); $$ = createBinaryExpNode($1, $3, "<"); }
    | expr GT expr          { log_rule("expr → expr GT expr", 31, 8); $$ = createBinaryExpNode($1, $3, ">"); }
    | expr LEQ expr         { log_rule("expr → expr LEQ expr", 31, 9); $$ = createBinaryExpNode($1, $3, "<="); }
    | expr GEQ expr         { log_rule("expr → expr GEQ expr", 31, 10); $$ = createBinaryExpNode($1, $3, ">="); }
    | expr AND expr         { log_rule("expr → expr AND expr", 31, 11); $$ = createBinaryExpNode($1, $3, "&&"); }
    | expr OR expr          { log_rule("expr → expr OR expr", 31, 12); $$ = createBinaryExpNode($1, $3, "||"); }
    | NOT expr %prec UNARY  { log_rule("expr → NOT expr", 31, 13); $$ = createUnaryExpNode($2, "!"); }
    | MINUS expr %prec UNARY{ log_rule("expr → MINUS expr", 31, 14); $$ = createUnaryExpNode($2, "-"); }
    | INC expr %prec UNARY  { log_rule("expr → INC expr", 31, 15); $$ = createUnaryExpNode($2, "PRE_INC"); }
    | DEC expr %prec UNARY  { log_rule("expr → DEC expr", 31, 16); $$ = createUnaryExpNode($2, "PRE_DEC"); }
    | expr INC %prec UNARY  { log_rule("expr → expr INC", 31, 17); $$ = createUnaryExpNode($1, "POST_INC"); }
    | expr DEC %prec UNARY  { log_rule("expr → expr DEC", 31, 18); $$ = createUnaryExpNode($1, "POST_DEC"); }
    | ID                    { log_rule("expr → ID", 31, 19); $$ = createTermExpNode(createIdRefNode($1)); } 
    | INT_LITERAL           { log_rule("expr → INT_LITERAL", 31, 20); $$ = createTermExpNode(createIntLiteralNode($1)); }
    | CHAR_LITERAL          { log_rule("expr → CHAR_LITERAL", 31, 21); $$ = createTermExpNode(createCharLiteralNode($1)); }
    | STR_LITERAL           { log_rule("expr → STR_LITERAL", 31, 22); $$ = createTermExpNode(createStrLiteralNode($1)); }
    | func_call             { log_rule("expr → func_call", 31, 23); $$ = $1; }
    | '(' expr ')'          { log_rule("expr → ( expr )", 31, 24); $$ = $2; }
    ;




%%


void yyerror(const char* s) {
    fprintf(stderr, "Error: %s at line %d, character %d\n", s, cur_line, cur_char);
    exit(0);
}

int main(int argc, char *argv[]){

    // Run time flags
    int exportAST_flag = 0;  
    int printAST_flag = 0;
    int printSymTable_flag = 0;
    int debug_flag = 0;
    int debug_sym_table = 0;
    int debug_ast_flag = 0;
    int debug_semantic_flag = 0;
    int debug_icg_flag = 0;
    init_logger();

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
        if (strcmp(argv[i], "--debug") == 0) {
            debug_flag = 1;
            isParserDebuggerOn = true;
        }
        if (strcmp(argv[i], "--debug-ast") == 0) {
            debug_ast_flag = 1;
        }
        if (strcmp(argv[i], "--debug-sym-table") == 0) {
            debug_sym_table = 1;
        }
        if (strcmp(argv[i], "--debug-semantic") == 0) {
            debug_semantic_flag = 1;
        }
        if (strcmp(argv[i], "--debug-icg") == 0) {
            debug_icg_flag = 1;
        }
    }

    // Turn on debuggers
    if(debug_flag){
        setASTDebugger();
        setSymTableDebugger();
        setSemanticDebugger();
        setICGDebugger();
    }

    if(debug_sym_table)     setSymTableDebugger();
    if(debug_ast_flag)      setASTDebugger();
    if(debug_semantic_flag) setSemanticDebugger();
    if(debug_icg_flag)      setICGDebugger();
  
    // Phase 1: Lexing + Parsing
    start_phase(PHASE_LEX_PARSE);

    symTable = createSymbolTable("global", NULL, 100);
    currentScope = symTable; // Initial current scope
    
    yyparse();
    end_phase(); // END Phase 1

    if (debug_flag || debug_semantic_flag) printf("\n------SEMANTIC ANALYSIS START------\n\n");
    SemanticStatus sem_stat = performSemanticAnalysis(root, symTable, brkCntListHEAD);
    if (debug_flag || debug_semantic_flag) printf("\n\n------SEMANTIC ANALYSIS END------\n\n");

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

    if(sem_stat == SEMANTIC_SUCCESS){
        printf("\nPARSING SUCCESS\n");

        startICG(root);

        printf("\nThreeAddressCode------------------------\n\n");
        printTAC();
    }
        
    
    printf("\n\n");

    

    close_logger();
    freeAST(root);
    freeSymbolTable(symTable);
}



// Wrapper function to create a program node with a list of statements
ASTNode* createProgramNode(ASTNode* stmt_list) {
    // Create a node for the root of the program
    ASTNode* programNode = createASTNode(NODE_PROGRAM, cur_line, cur_char);

    programNode->program_data.stmt_list = stmt_list;
    programNode->program_data.scope = symTable;
    
    return programNode;
}

ASTNode* createStmtListNode(ASTNode* stmt_list, ASTNode* stmt) {
    ASTNode* node = createASTNode(NODE_STMT_LIST, cur_line, cur_char);
    node->stmt_list_data.stmt_list = stmt_list; // Pointer to the next statement or NULL
    node->stmt_list_data.stmt = stmt;
    return node;
}

ASTNode* createBlockStmtNode(ASTNode* stmt_list){
    ASTNode* node = createASTNode(NODE_BLOCK_STMT, cur_line, cur_char);
    node->block_stmt_data.stmt_list = stmt_list;
    
    return node;
}



ASTNode* createReturnNode(ASTNode* return_value){
    ASTNode* node = createASTNode(NODE_RETURN, cur_line, cur_char);
    node->return_data.return_value = return_value;
    node->return_data.associated_node = NULL;
    return node;
}

ASTNode* createBreakNode(){
    ASTNode* node = createASTNode(NODE_BREAK_STMT, cur_line, cur_char);
    node->break_continue_stmt_data.associated_loop_node = NULL;

    // Add to the list
    BrkCntStmtsList* list = (BrkCntStmtsList*)malloc(sizeof(BrkCntStmtsList));
    list->node = node;
    list->next = NULL;
    if(!brkCntListHEAD){
        brkCntList = list;
        brkCntListHEAD = list;
    }else{
        brkCntList->next = list;
        brkCntList = brkCntList->next;
    }            

    return node;
}
ASTNode* createContinueNode(){
    ASTNode* node = createASTNode(NODE_CONTINUE_STMT, cur_line, cur_char);
    node->break_continue_stmt_data.associated_loop_node = NULL;

    // Add to the list
    BrkCntStmtsList* list = (BrkCntStmtsList*)malloc(sizeof(BrkCntStmtsList));
    list->node = node;
    list->next = NULL;
    if(!brkCntListHEAD){
        brkCntList = list;
        brkCntListHEAD = list;
    }else{
        brkCntList->next = list;
        brkCntList = brkCntList->next;
    } 

    return node;
}


// EXPRESSIONS

ASTNode* createBinaryExpNode(ASTNode* left, ASTNode* right, const char* op) {
    ASTNode* node = createASTNode(NODE_EXPR_BINARY, cur_line, cur_char);
    node->expr_data.left = left;
    node->expr_data.right = right;
    node->expr_data.op = strdup(op);  // Store the operation
    return node;
}

ASTNode* createUnaryExpNode(ASTNode* left, const char* op){
    ASTNode* node = createASTNode(NODE_EXPR_UNARY, cur_line, cur_char);
    node->expr_data.left = left;
    node->expr_data.right = NULL;
    node->expr_data.op = strdup(op);  // Store the operation
    return node;
}

ASTNode* createTermExpNode(ASTNode* term){
    ASTNode* node = createASTNode(NODE_EXPR_TERM, cur_line, cur_char);
    node->expr_data.left = term;
    node->expr_data.right = NULL;
    node->expr_data.op = NULL;
    return node;
}



// IDENTIFIERS

ASTNode* createIdentifierNode(const char* id, char* type) {
    ASTNode* node = createASTNode(NODE_ID, cur_line, cur_char);

    symbol* sym = createSymbol(id, type, currentScope, -1, 0, cur_line, cur_char);
    node->id_data.sym = sym;
    addSymbol(currentScope, sym);
    return node;
}

ASTNode* createFucnIdNode(const char* id, ASTNode* type_spec){
    ASTNode* node = createASTNode(NODE_ID, cur_line, cur_char);
    char* type = (char*)type_spec->type_data.type;

    symbol* sym = createSymbol(id, type, currentScope, -1, 1, cur_line, cur_char);
    node->id_data.sym = sym;
    addSymbol(currentScope, sym);
    return node;  
}

ASTNode* createIdRefNode(const char* id){
    ASTNode* node = createASTNode(NODE_ID_REF, cur_line, cur_char);
    node->id_ref_data.name = id;
    node->id_ref_data.ref = NULL;
    node->id_ref_data.scope = currentScope;
    node->id_ref_data.line_no = cur_line;
    node->id_ref_data.char_no = cur_char;
    return node;
}



// LITERALS

ASTNode* createIntLiteralNode(int i){
    ASTNode* node = createASTNode(NODE_INT_LITERAL, cur_line, cur_char);
    node->literal_data.value.int_value = i;
    return node;
}
ASTNode* createCharLiteralNode(char c){
    ASTNode* node = createASTNode(NODE_CHAR_LITERAL, cur_line, cur_char);
    node->literal_data.value.char_value = c;
    return node;
}
ASTNode* createStrLiteralNode(const char* s){
    ASTNode* node = createASTNode(NODE_STR_LITERAL, cur_line, cur_char);
    node->literal_data.value.str_value = s;
    return node;
}



// DECLARATIONS, VARIABLES and ASSIGNMENTS

ASTNode* createDeclNode(ASTNode* type_spec, ASTNode* var_list){
    ASTNode* node = createASTNode(NODE_DECL, cur_line, cur_char);
    node->decl_data.type_spec = type_spec;
    node->decl_data.var_list = var_list;

    return node;
}

ASTNode* createTypeNode(char* type){
    ASTNode* node = createASTNode(NODE_TYPE_SPEC, cur_line, cur_char);
    node->type_data.type = type;
    return node;
}

ASTNode* createVarListNode(ASTNode* var_list, ASTNode* var){
    ASTNode* node = createASTNode(NODE_VAR_LIST, cur_line, cur_char);
    node->var_list_data.var_list = var_list;
    node->var_list_data.var = var;

    return node;
}

ASTNode* createVarNode(const char* id) {
    ASTNode* node = createASTNode(NODE_VAR, cur_line, cur_char);
    node->var_data.id = createIdentifierNode(id, NULL);  // Simple variable
    node->var_data.value = NULL;
    return node;
}

ASTNode* createVarAssgnNode(const char* id, ASTNode* value){
   ASTNode* node = createASTNode(NODE_VAR, cur_line, cur_char);
   node->var_data.id = createIdentifierNode(id, NULL);
   node->var_data.value = value;
   return node;
}

ASTNode* createAssgnNode(const char* id, ASTNode* value){
    ASTNode* node = createASTNode(NODE_ASSGN, cur_line, cur_char);
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
                if (isParserDebuggerOn) printLog("Assigning type: %s to sym: %s in Var List", type, sym->name);
                sym->type = strdup(type); // Assign the type to the variable's symbol
                log_assgn_sym_type(sym->name, sym->scope->table_id, type);
            }
        }

    }
}



// IF ELSE 

ASTNode* createIfNode(ASTNode* cond, ASTNode* if_branch) {
    ASTNode* node = createASTNode(NODE_IF, cur_line, cur_char);

    // Implicit creation of cond & branch nodes

    ASTNode* node_cond = createASTNode(NODE_IF_COND, cur_line, cur_char);
    node_cond->if_cond_data.cond = cond;
    node->if_else_data.condition = node_cond;

    ASTNode* node_if_branch = createASTNode(NODE_IF_BRANCH, cur_line, cur_char);
    node_if_branch->if_else_branch.branch = if_branch;
    node->if_else_data.if_branch = node_if_branch;

    node->if_else_data.else_branch = NULL;
    return node;
}

ASTNode* createIfElseNode(ASTNode* cond, ASTNode* if_branch, ASTNode* else_branch){
    ASTNode* node = createASTNode(NODE_IF_ELSE, cur_line, cur_char);

    // Implicit creation of cond & branch nodes

    ASTNode* node_cond = createASTNode(NODE_IF_COND, cur_line, cur_char);
    node_cond->if_cond_data.cond = cond;
    node->if_else_data.condition = node_cond;

    ASTNode* node_if_branch = createASTNode(NODE_IF_BRANCH, cur_line, cur_char);
    node_if_branch->if_else_branch.branch = if_branch;
    node->if_else_data.if_branch = node_if_branch;
   
    ASTNode* node_else_branch = createASTNode(NODE_ELSE_BRANCH, cur_line, cur_char);
    node_else_branch->if_else_branch.branch = else_branch;
    node->if_else_data.else_branch = node_else_branch;
    
    return node;
}

ASTNode* createWhileNode(ASTNode* cond, ASTNode* body){
    ASTNode* node = createASTNode(NODE_WHILE, cur_line, cur_char);

    // Implict creation of cond and branch nodes
    ASTNode* node_while_cond = createASTNode(NODE_WHILE_COND, cur_line, cur_char);
    node_while_cond->while_cond_data.cond = cond;

    ASTNode* node_while_body = createASTNode(NODE_WHILE_BODY, cur_line, cur_char);
    node_while_body->while_body_data.body = body;

    node->while_data.condition = node_while_cond;
    node->while_data.while_body = node_while_body;

    return node;

}

ASTNode* createForNode(ASTNode* init, ASTNode* cond, ASTNode* updation, ASTNode* body){
    ASTNode* node = createASTNode(NODE_FOR, cur_line, cur_char);
    

    // Implicit creation of init, cond and updation nodes;
    ASTNode* node_for_init = createASTNode(NODE_FOR_INIT, cur_line, cur_char);
    node_for_init->for_init_data.init = init;

    ASTNode* node_for_cond = createASTNode(NODE_FOR_COND, cur_line, cur_char); 
    node_for_cond->for_cond_data.cond = cond;

    ASTNode* node_for_upd = createASTNode(NODE_FOR_UPDATION, cur_line, cur_char);
    node_for_upd->for_updation_data.updation = updation;

    ASTNode* node_for_body = createASTNode(NODE_FOR_BODY, cur_line, cur_char);
    node_for_body->for_body_data.body = body;

    node->for_data.init = node_for_init;
    node->for_data.condition = node_for_cond;
    node->for_data.updation = node_for_upd;
    node->for_data.body = node_for_body;

    return node;
}


ASTNode* createCommaExprList(ASTNode* expr_list, ASTNode* expr_list_item){
    ASTNode* node = createASTNode(NODE_EXPR_COMMA_LIST, cur_line, cur_char);

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
    ASTNode* node = createASTNode(NODE_FUNC_DECL, cur_line, cur_char);

    if (func_id >= 100) {
        fprintf(stderr, "Max functions limit exceded\n");
        exit(0);
    }

    node->func_decl_data.global_id = func_id++; 
    node->func_decl_data.id = id;
    id->id_data.sym->func_node = node;

    node->func_decl_data.params = params;
    node->func_decl_data.param_count = countParams(params);
    /* printf("Params count: %d\n", node->func_decl_data.param_count); */

    // Implicit creation of Function body node
    ASTNode* body_node = createASTNode(NODE_FUNC_BODY, cur_line, cur_char);
    body_node->func_body_data.body = body;

    node->func_decl_data.body = body_node;
    node->func_decl_data.scope = currentScope;

    return node;
}

ASTNode* createParamsListNode(ASTNode* params_list, ASTNode* param){
    ASTNode* node = createASTNode(NODE_PARAM_LIST, cur_line, cur_char);

    node->param_list_data.param_list = params_list;
    node->param_list_data.param = param;

    return node;
}

ASTNode* createParamNode(ASTNode* type_spec, const char* id){
    ASTNode* node = createASTNode(NODE_PARAM, cur_line, cur_char);
    char* type = (char*)type_spec->type_data.type;
    node->param_data.type_spec = type_spec;
    node->param_data.id = createIdentifierNode(id, type);
    return node;
}

int countArgs(ASTNode* arg_list) {
    if (arg_list == NULL) return 0;

    // Check if the node is a parameter list node
    if (arg_list->type == NODE_ARG_LIST) {
        return countArgs(arg_list->arg_list_data.arg_list) + countArgs(arg_list->arg_list_data.arg);
    }

    return 1;
}

ASTNode* createFuncCallNode(const char* id, ASTNode* arg_list){
    ASTNode* node = createASTNode(NODE_FUNC_CALL, cur_line, cur_char);


    node->func_call_data.id = createIdRefNode(id);
    node->func_call_data.arg_list = arg_list;
    node->func_call_data.arg_count = countArgs(arg_list);
    return node;

}

ASTNode* createArgListNode(ASTNode* arg_list, ASTNode* arg){
    ASTNode* node = createASTNode(NODE_ARG_LIST, cur_line, cur_char);

    node->arg_list_data.arg_list = arg_list;
    node->arg_list_data.arg = createArgNode(arg);

    return node;
}

ASTNode* createArgNode(ASTNode* arg){
    ASTNode* node = createASTNode(NODE_ARG, cur_line, cur_char);

    node->arg_data.arg = arg;
    return node;
}