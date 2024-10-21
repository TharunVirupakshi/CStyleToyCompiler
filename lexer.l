%{
    #include "ast.h"
    #include "y.tab.h"
    #define COUNT() (cur_char+=yyleng) 

    int cur_line = 1;
    int cur_char = 1;

%}

alpha [a-zA-Z]
digit [0-9]


%%

"if"        { COUNT(); return IF; }
"else"      { COUNT(); return ELSE; }
"for"       { COUNT(); return FOR; }
"while"     { COUNT(); return WHILE; }
"return"    { COUNT(); return RETURN; }
"int"       { COUNT(); return INT; }
"float"     { COUNT(); return FLOAT; }
"char"      { COUNT(); return CHAR; }
"string"    { COUNT(); return STRING;}
"void"      { COUNT(); return VOID; }

({alpha}|_)({alpha}|_|{digit})*     { COUNT(); yylval.strval = strdup(yytext); return ID;}
[-]?{digit}+                        { COUNT(); yylval.ival = atoi(yytext); return INT_LITERAL;}

\"[^\"]*\"                          { COUNT(); yylval.strval = strdup(yytext); return STR_LITERAL; }  // Strings: any characters between double quotes
\'[^\']\'                           { COUNT(); yylval.ival = yytext[1]; return CHAR_LITERAL; }           // Characters: single character within single quotes

"+"     { COUNT(); return PLUS; }
"-"     { COUNT(); return MINUS; }
"*"     { COUNT(); return MULT; }
"/"     { COUNT(); return DIV; }
"++"    { COUNT(); return INC;}
"--"    { COUNT(); return DEC;}

"=="    { COUNT(); return EQ; }
"!="    { COUNT(); return NEQ; }
"<="    { COUNT(); return LEQ; }
">="    { COUNT(); return GEQ; }
"<"     { COUNT(); return LT; }
">"     { COUNT(); return GT; }

"&&"    { COUNT(); return AND;}
"||"    { COUNT(); return OR;}
"!"     { COUNT(); return NOT;}

"="     { COUNT(); return ASSIGN; }

\/\/.*                      { COUNT(); /* Ignore single line comments */ ; }
\/\*([^*]|\*+[^\/])*\*\/    { COUNT(); /* Ignore multi-line comments */ ; }
\n                          { cur_line++; cur_char=1; }
[ \t\r]+                    { COUNT(); }

.                           { COUNT(); return yytext[0]; }
%%

int yywrap() {
    return 1;
}