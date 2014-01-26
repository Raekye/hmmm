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
	SExpression* single_element;
	SExpression* multiple_elements;
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
%type <single_element> single_element
%type <multiple_elements> multiple_elements

%%

input
	: expr { *expression = $1; }
	;

expr
	: TOKEN_ADD single_element multiple_elements { $$ = createOperation(eADD, $2, $3); }
	| TOKEN_SUBTRACT single_element multiple_elements { $$ = createOperation(eSUBTRACT, $2, $3); }
	| TOKEN_MULTIPLY single_element multiple_elements { $$ = createOperation(eMULTIPLY, $2, $3); }
	| TOKEN_DIVIDE single_element multiple_elements { $$ = createOperation(eDIVIDE, $2, $3); }
	;

single_element
	: TOKEN_LPAREN expr TOKEN_RPAREN { $$ = $2; }
	| TOKEN_NUMBER { $$ = createNumber($1); }
	;

multiple_elements
	: single_element multiple_elements { $$ = appendVector($1, $2); }
	| single_element { $$ = createVector($1); }
	;