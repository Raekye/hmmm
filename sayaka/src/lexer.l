%{

#include "node.h"
#include "parser.h"
#include <cstdio>

// yytext, yyleng globals (http://flex.sourceforge.net/manual/Matching.html)
#define SAVE_TOKEN yylval->str = new std::string(yytext, yyleng)

%}

/* (http://flex.sourceforge.net/manual/Code_002dLevel-And-API-Options.html) */
%option reentrant
%option noyywrap
%option never-interactive
%option nounistd
%option bison-bridge
%option bison-locations

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
