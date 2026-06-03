%{
    #include "symTable.h"
    #include "ast.h"
    #include "parser.tab.h"
    #include "logger.h"

    extern YYLTYPE yylloc;

    #define UPDATE_LOC()                               \
    do {                                               \
        yylloc.first_line = cur_line;                  \
        yylloc.first_column = cur_char;                \
                                                       \
        cur_char += yyleng;                            \
                                                       \
        yylloc.last_line = cur_line;                   \
        yylloc.last_column = cur_char - 1;             \
    } while(0)

    int cur_line = 1;
    int cur_char = 1;

    void log_token(const char* tokenName, const char* value) {
        Step s;
        s.type = LEX_READ_TOKEN;
        s.readToken.tokenName = tokenName;
        s.readToken.value = value;
        s.readToken.line_no = cur_line;
        s.readToken.char_no = cur_char;
        log_step(s);
    } 

%}

alpha [a-zA-Z]
digit [0-9]


%%

"if"        { UPDATE_LOC(); log_token("KEYWORD", "IF");       return IF; }
"else"      { UPDATE_LOC(); log_token("KEYWORD", "ELSE");     return ELSE; }
"for"       { UPDATE_LOC(); log_token("KEYWORD", "FOR");      return FOR; }
"while"     { UPDATE_LOC(); log_token("KEYWORD", "WHILE");    return WHILE; }
"return"    { UPDATE_LOC(); log_token("KEYWORD", "RETURN");   return RETURN; }
"break"     { UPDATE_LOC(); log_token("KEYWORD", "BREAK");    return BREAK; }
"continue"  { UPDATE_LOC(); log_token("KEYWORD", "CONTINUE"); return CONTINUE; }
"int"       { UPDATE_LOC(); log_token("KEYWORD", "INT");      return INT; }
"float"     { UPDATE_LOC(); log_token("KEYWORD", "FLOAT");    return FLOAT; }
"char"      { UPDATE_LOC(); log_token("KEYWORD", "CHAR");     return CHAR; }
"string"    { UPDATE_LOC(); log_token("KEYWORD", "STRING");   return STRING; }
"void"      { UPDATE_LOC(); log_token("KEYWORD", "VOID");     return VOID; }

({alpha}|_)({alpha}|_|{digit})* {
    UPDATE_LOC();
    yylval.strval = strdup(yytext);
    log_token("ID", yytext);
    return ID;
}

[-]?{digit}+ {
    UPDATE_LOC();
    yylval.ival = atoi(yytext);
    log_token("INT_LITERAL", yytext);
    return INT_LITERAL;
}

\"[^\"]*\" {
    UPDATE_LOC();
    yylval.strval = strdup(yytext);
    log_token("STR_LITERAL", yytext);
    return STR_LITERAL;
}

\'[^\']\' {
    UPDATE_LOC();
    yylval.ival = yytext[1];
    log_token("CHAR_LITERAL", yytext);
    return CHAR_LITERAL;
}

"+"     { UPDATE_LOC(); log_token("OPERATOR", "PLUS");     return PLUS; }
"-"     { UPDATE_LOC(); log_token("OPERATOR", "MINUS");    return MINUS; }
"*"     { UPDATE_LOC(); log_token("OPERATOR", "MULT");     return MULT; }
"/"     { UPDATE_LOC(); log_token("OPERATOR", "DIV");      return DIV; }
"++"    { UPDATE_LOC(); log_token("OPERATOR", "INC");      return INC; }
"--"    { UPDATE_LOC(); log_token("OPERATOR", "DEC");      return DEC; }

"=="    { UPDATE_LOC(); log_token("OPERATOR", "EQ");       return EQ; }
"!="    { UPDATE_LOC(); log_token("OPERATOR", "NEQ");      return NEQ; }
"<="    { UPDATE_LOC(); log_token("OPERATOR", "LEQ");      return LEQ; }
">="    { UPDATE_LOC(); log_token("OPERATOR", "GEQ");      return GEQ; }
"<"     { UPDATE_LOC(); log_token("OPERATOR", "LT");       return LT; }
">"     { UPDATE_LOC(); log_token("OPERATOR", "GT");       return GT; }

"&&"    { UPDATE_LOC(); log_token("OPERATOR", "AND");      return AND; }
"||"    { UPDATE_LOC(); log_token("OPERATOR", "OR");       return OR; }
"!"     { UPDATE_LOC(); log_token("OPERATOR", "NOT");      return NOT; }

"="     { UPDATE_LOC(); log_token("OPERATOR", "ASSIGN");   return ASSIGN; }

\/\/.*                      { UPDATE_LOC(); log_token("SINGLE_LINE_COMMENT", yytext); }
\/\*([^*]|\*+[^\/])*\*\/    { UPDATE_LOC(); log_token("MULTI_LINE_COMMENT", yytext); }
\n                          { 
                                yylloc.first_line = cur_line; yylloc.first_column = cur_char;
                                cur_line++; cur_char = 1; 
                                yylloc.last_line = cur_line - 1; yylloc.last_column = 1; 
                                log_token("NEW_LINE", "");
                            }
[ \t\r]+                    { UPDATE_LOC(); log_token("WHITE_SPACE", ""); /* skip whitespace */ }

.                           { UPDATE_LOC(); log_token("READ_CHARACTER", yytext); return yytext[0]; }
%%

int yywrap() {
    return 1;
}
