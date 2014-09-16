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
LBRACE "{"
RBRACE "}"
SEMICOLON ";"
ADD "+"
MULTIPLY "*"
SUBTRACT "-"
DIVIDE "/"
POW "*\\*"
EQUALS "="

IDENTIFIER [a-z_][a-zA-Z0-9_]*
TYPE_NAME [A-Z][a-zA-Z0-9_]*

NUMBER [0-9][0-9_]*(\.[0-9][0-9_]*)?(E[+-]?[0-9]+)?
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
{LBRACE} { return TOKEN_LBRACE; }
{RBRACE} { return TOKEN_RBRACE; }
{SEMICOLON} { return TOKEN_SEMICOLON; }
{EQUALS} { return TOKEN_EQUALS; }
{IDENTIFIER} { SAVE_TOKEN; return TOKEN_IDENTIFIER; }
{TYPE_NAME} { SAVE_TOKEN; return TOKEN_TYPE_NAME; }
. { printf("Unknown token!"); }

%%
