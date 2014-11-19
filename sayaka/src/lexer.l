%{

#include <iostream>
#include "parser.h"

#define SAVE_TOKEN yylval->str = new std::string(yytext, yyleng)

%}

%option reentrant
%option debug
%option noyywrap
%option never-interactive
%option nounistd
%option yylineno
%option bison-bridge
%option bison-locations

LPAREN "("
RPAREN ")"
LBRACE "{"
RBRACE "}"
LBRACKET "\["
RBRACKET "\]"
SEMICOLON ";"
ADD "+"
MULTIPLY "*"
SUBTRACT "-"
DIVIDE "/"
POW "\\*\\*"
ASSIGN "="
COMMA ","
EQ "=="
NEQ "!="
LT "<"
GT ">"
LEQ "<="
GEQ ">="
COLON ":"

IDENTIFIER [a-z_][a-zA-Z0-9_]*
TYPE_NAME [A-Z][a-zA-Z0-9_]*

NUMBER [0-9][0-9_]*(\.[0-9][0-9_]*)?(E[+-]?[0-9]+)?
WS [ \r\n\t]+

%%

{WS} { /* pass */ }

{MULTIPLY} { return TOKEN_MULTIPLY; }
{DIVIDE} { return TOKEN_DIVIDE; }
{ADD} { return TOKEN_ADD; }
{SUBTRACT} { return TOKEN_SUBTRACT; }
{POW} { return TOKEN_POW; }
{LPAREN} { return TOKEN_LPAREN; }
{RPAREN} { return TOKEN_RPAREN; }
{LBRACE} { return TOKEN_LBRACE; }
{RBRACE} { return TOKEN_RBRACE; }
{LBRACKET} { return TOKEN_LBRACKET; }
{RBRACKET} { return TOKEN_RBRACKET; }
{SEMICOLON} { return TOKEN_SEMICOLON; }
{ASSIGN} { return TOKEN_ASSIGN; }
{COMMA} { return TOKEN_COMMA; }
{EQ} { return TOKEN_EQ; }
{NEQ} { return TOKEN_NEQ; }
{LT} { return TOKEN_LT; }
{GT} { return TOKEN_GT; }
{LEQ} { return TOKEN_LEQ; }
{GEQ} { return TOKEN_GEQ; }
{COLON} { return TOKEN_COLON; }
if { return TOKEN_IF; }
else { return TOKEN_ELSE; }
var { return TOKEN_VAR; }
val { return TOKEN_VAL; }

{NUMBER} { SAVE_TOKEN; return TOKEN_NUMBER; }
{IDENTIFIER} { SAVE_TOKEN; return TOKEN_IDENTIFIER; }
{TYPE_NAME} { SAVE_TOKEN; return TOKEN_TYPE_NAME; }

. { std::cout << "Unknown token!" << std::endl; }

%%
