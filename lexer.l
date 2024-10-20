%{
    #include "y.tab.h"
%}

alpha [a-zA-Z]
digit [0-9]


%%

"if"        { return IF; }
"else"      { return ELSE; }
"for"       { return FOR; }
"while"     { return WHILE; }
"return"    { return RETURN; }
"int"       { return INT; }
"float"     { return FLOAT; }
"char"      { return CHAR; }
"string"    { return STRING;}
"void"      { return VOID; }

({alpha}|_)({alpha}|_|{digit})*     {yylval.strval = strdup(yytext); return ID;}
[-]?{digit}+                        {yylval.ival = atoi(yytext); return INT_LITERAL;}

\"[^\"]*\"                          { yylval.strval = strdup(yytext); return STR_LITERAL; }  // Strings: any characters between double quotes
\'[^\']\'                           { yylval.ival = yytext[1]; return CHAR_LITERAL; }           // Characters: single character within single quotes

"+"     { return PLUS; }
"-"     { return MINUS; }
"*"     { return MULT; }
"/"     { return DIV; }
"++"    { return INC;}
"--"    { return DEC;}

"=="    { return EQ; }
"!="    { return NEQ; }
"<="    { return LEQ; }
">="    { return GEQ; }
"<"     { return LT; }
">"     { return GT; }

"="     { return ASSIGN; }

\/\/.*                      { /* Ignore single line comments */ ; }
\/\*([^*]|\*+[^\/])*\*\/    { /* Ignore multi-line comments */ ; }

[ \t\n\r]+                  { /* Ignore whitespace */ }

.                           { return yytext[0]; }
%%

int yywrap() {
    return 1;
}