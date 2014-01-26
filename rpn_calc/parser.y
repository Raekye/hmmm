%{

#include "expression.h"
#include "parser.h"
#include "lexer.h"

int yyerror(SExpression** expr, yyscan_t scanner, const char* msg) {
	// error handling
}

%}

%code requires {

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

}

%output "parser.c"
%defines "parser.h"

%define api.pure
%lex-param { yyscan_t scanner }
%parse-param { SExpression** expression }
%parse-param { yyscan_t scanner }

%union {
	int value;
	SExpression* expression;
}

%left '+' TOKEN_ADD
%left '*' TOKEN_MULTIPLY
%left '/' TOKEN_DIVIDE
%left '-' TOKEN_SUBTRACT

%token TOKEN_LPAREN
%token TOKEN_RPAREN
%token TOKEN_ADD
%token TOKEN_MULTIPLY
%token TOKEN_DIVIDE
%token TOKEN_SUBTRACT
%token <value> TOKEN_NUMBER

%type <expression> expr

%%

input
	: expr { *expression = $1; }
	;

expr
	: expr TOKEN_MULTIPLY expr { $$ = createOperation(eMULTIPLY, $1, $3); }
	| expr TOKEN_DIVIDE expr { $$ = createOperation(eDIVIDE, $1, $3); }
	| expr TOKEN_ADD expr { $$ = createOperation(eADD, $1, $3); }
	| expr TOKEN_SUBTRACT expr { $$ = createOperation(eSUBTRACT, $1, $3); }
	| TOKEN_LPAREN expr TOKEN_RPAREN { $$ = $2; }
	| TOKEN_NUMBER { $$ = createNumber($1); }
	;