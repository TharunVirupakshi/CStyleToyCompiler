/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ID = 258,
     STR_LITERAL = 259,
     INT_LITERAL = 260,
     CHAR_LITERAL = 261,
     IF = 262,
     ELSE = 263,
     WHILE = 264,
     FOR = 265,
     RETURN = 266,
     INT = 267,
     FLOAT = 268,
     CHAR = 269,
     VOID = 270,
     STRING = 271,
     PLUS = 272,
     MINUS = 273,
     MULT = 274,
     DIV = 275,
     INC = 276,
     DEC = 277,
     EQ = 278,
     NEQ = 279,
     LEQ = 280,
     GEQ = 281,
     LT = 282,
     GT = 283,
     ASSIGN = 284,
     UNARY = 285
   };
#endif
/* Tokens.  */
#define ID 258
#define STR_LITERAL 259
#define INT_LITERAL 260
#define CHAR_LITERAL 261
#define IF 262
#define ELSE 263
#define WHILE 264
#define FOR 265
#define RETURN 266
#define INT 267
#define FLOAT 268
#define CHAR 269
#define VOID 270
#define STRING 271
#define PLUS 272
#define MINUS 273
#define MULT 274
#define DIV 275
#define INC 276
#define DEC 277
#define EQ 278
#define NEQ 279
#define LEQ 280
#define GEQ 281
#define LT 282
#define GT 283
#define ASSIGN 284
#define UNARY 285




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 11 "parser.y"
{
    int ival;
    char* strval;
}
/* Line 1529 of yacc.c.  */
#line 114 "y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

