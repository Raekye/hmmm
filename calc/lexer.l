%{

#include "node.h"
#include "parser.hpp"
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
RAISE "*\\*"

DIGIT [0-9]

NUMBER {DIGIT}+
WS [ \r\n\t]*

%%

{WS} { /* pass */ }
{NUMBER} { SAVE_TOKEN; return TOKEN_NUMBER; }

{MULTIPLY} { return TOKEN_MULTIPLY; }
{DIVIDE} { return TOKEN_DIVIDE; }
{ADD} { return TOKEN_ADD; }
{SUBTRACT} { return TOKEN_SUBTRACT; }
{RAISE} { return TOKEN_RAISE; }
{LPAREN} { return TOKEN_LPAREN; }
{RPAREN} { return TOKEN_RPAREN; }
{SEMICOLON} { return TOKEN_SEMICOLON; }
. { printf("Unknown token!"); }

%%