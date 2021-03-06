%{

#include "node.h"
#include "parser.h"
#include <cstdio>

#define SAVE_TOKEN yylval->str = new std::string(yytext, yyleng)

%}

%option reentrant noyywrap never-interactive nounistd
%option bison-bridge

LPAREN "("
RPAREN ")"
SEMICOLON ";"
ADD "+"
MULTIPLY "*"
SUBTRACT "-"
DIVIDE "/"
POW "*\\*"
EQUALS "="

DIGIT [0-9]
IDENTIFIER [a-zA-Z_][a-zA-Z0-9_]*

NUMBER {DIGIT}+
WS [ \r\n\t]+

%%

{WS} { /* pass */ }
{NUMBER} { SAVE_TOKEN; return TOKEN_NUMBER; }

{MULTIPLY} { return TOKEN_MULTIPLY; }
{DIVIDE} { return TOKEN_DIVIDE; }
{ADD} { return TOKEN_ADD; }
{SUBTRACT} { return TOKEN_SUBTRACT; }
{POW} { return TOKEN_POW; }
{LPAREN} { return TOKEN_LPAREN; }
{RPAREN} { return TOKEN_RPAREN; }
{SEMICOLON} { return TOKEN_SEMICOLON; }
{IDENTIFIER} { SAVE_TOKEN; return TOKEN_IDENTIFIER; }
{EQUALS} { return TOKEN_EQUALS; }
. { printf("Unknown token!"); }

%%
