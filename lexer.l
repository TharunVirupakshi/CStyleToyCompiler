%{
    #include "symTable.h"
    #include "ast.h"
    #include "parser.tab.h"
    #include "logger.h"

    #define COUNT() (cur_char+=yyleng)

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

"if"        { COUNT(); log_token("KEYWORD", "IF");       return IF; }
"else"      { COUNT(); log_token("KEYWORD", "ELSE");     return ELSE; }
"for"       { COUNT(); log_token("KEYWORD", "FOR");      return FOR; }
"while"     { COUNT(); log_token("KEYWORD", "WHILE");    return WHILE; }
"return"    { COUNT(); log_token("KEYWORD", "RETURN");   return RETURN; }
"break"     { COUNT(); log_token("KEYWORD", "BREAK");    return BREAK; }
"continue"  { COUNT(); log_token("KEYWORD", "CONTINUE"); return CONTINUE; }
"int"       { COUNT(); log_token("KEYWORD", "INT");      return INT; }
"float"     { COUNT(); log_token("KEYWORD", "FLOAT");    return FLOAT; }
"char"      { COUNT(); log_token("KEYWORD", "CHAR");     return CHAR; }
"string"    { COUNT(); log_token("KEYWORD", "STRING");   return STRING; }
"void"      { COUNT(); log_token("KEYWORD", "VOID");     return VOID; }

({alpha}|_)({alpha}|_|{digit})* {
    COUNT();
    yylval.strval = strdup(yytext);
    log_token("ID", yytext);
    return ID;
}

[-]?{digit}+ {
    COUNT();
    yylval.ival = atoi(yytext);
    log_token("INT_LITERAL", yytext);
    return INT_LITERAL;
}

\"[^\"]*\" {
    COUNT();
    yylval.strval = strdup(yytext);
    log_token("STR_LITERAL", yytext);
    return STR_LITERAL;
}

\'[^\']\' {
    COUNT();
    yylval.ival = yytext[1];
    log_token("CHAR_LITERAL", yytext);
    return CHAR_LITERAL;
}

"+"     { COUNT(); log_token("OPERATOR", "PLUS");     return PLUS; }
"-"     { COUNT(); log_token("OPERATOR", "MINUS");    return MINUS; }
"*"     { COUNT(); log_token("OPERATOR", "MULT");     return MULT; }
"/"     { COUNT(); log_token("OPERATOR", "DIV");      return DIV; }
"++"    { COUNT(); log_token("OPERATOR", "INC");      return INC; }
"--"    { COUNT(); log_token("OPERATOR", "DEC");      return DEC; }

"=="    { COUNT(); log_token("OPERATOR", "EQ");       return EQ; }
"!="    { COUNT(); log_token("OPERATOR", "NEQ");      return NEQ; }
"<="    { COUNT(); log_token("OPERATOR", "LEQ");      return LEQ; }
">="    { COUNT(); log_token("OPERATOR", "GEQ");      return GEQ; }
"<"     { COUNT(); log_token("OPERATOR", "LT");       return LT; }
">"     { COUNT(); log_token("OPERATOR", "GT");       return GT; }

"&&"    { COUNT(); log_token("OPERATOR", "AND");      return AND; }
"||"    { COUNT(); log_token("OPERATOR", "OR");       return OR; }
"!"     { COUNT(); log_token("OPERATOR", "NOT");      return NOT; }

"="     { COUNT(); log_token("OPERATOR", "ASSIGN");   return ASSIGN; }

\/\/.*                      { COUNT(); log_token("SINGLE_LINE_COMMENT", yytext); }
\/\*([^*]|\*+[^\/])*\*\/    { COUNT(); log_token("MULTI_LINE_COMMENT", yytext); }
\n                          { cur_line++; cur_char=1; log_token("NEW_LINE", ""); }
[ \t\r]+                    { COUNT(); log_token("WHITE_SPACE", ""); /* skip whitespace */ }

.                           { COUNT(); log_token("READ_CHARACTER", yytext); return yytext[0]; }
%%

int yywrap() {
    return 1;
}
