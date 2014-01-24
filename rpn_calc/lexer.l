%{

#include "expression.h"
#include "parser.h"
#include <stdio.h>

%}

%option outfile="lexer.c" header-file="lexer.h"
%option reentrant noyywrap never-interactive nounistd
%option bison-bridge

LPAREN "("
RPAREN ")"
ADD "+"
MULTIPLY "*"
SUBTRACT "-"
DIVIDE "/"

NUMBER [0-9]+
WS [ \r\n\t]*

%%

{WS} { /* pass */ }
{NUMBER} { sscanf(yytext, "%d", &yylval->value); return TOKEN_NUMBER; }

{MULTIPLY} { return TOKEN_MULTIPLY; }
{DIVIDE} { return TOKEN_DIVIDE; }
{ADD} { return TOKEN_ADD; }
{SUBTRACT} { return TOKEN_SUBTRACT; }
{LPAREN} { return TOKEN_LPAREN; }
{RPAREN} { return TOKEN_RPAREN; }

%%

int yyerror(const char* msg) {
	fprintf(stderr, "Error: %s\n", msg);
	return 0;
}