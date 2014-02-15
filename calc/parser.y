%{

#include "node.h"
#include "parser.hpp"
#include "lexer.h"
#include <cstdio>

void yyerror(NExpression**, yyscan_t scanner, const char *s) {
	fprintf(stderr, "YYERROR: %s!", s);
}

NExpression* programBlock;

%}

%code requires {

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

}

%define api.pure
%lex-param   { yyscan_t scanner }
%parse-param { NExpression** expression }
%parse-param { yyscan_t scanner }

%union {
	Node* node;
	NExpression* expr;
	std::string* str;
}

%left TOKEN_ADD TOKEN_SUBTRACT TOKEN_MULTIPLY TOKEN_DIVIDE TOKEN_RAISE

%token TOKEN_LPAREN TOKEN_RPAREN TOKEN_SEMICOLON
%token <token> TOKEN_ADD TOKEN_MULTIPLY TOKEN_DIVIDE TOKEN_SUBTRACT TOKEN_RAISE
%token <str> TOKEN_NUMBER

%type <expr> program expr number_expr number_expr2 number_expr3 number_expr4 number

%start program

%%

program
	: expr { *expression = $1; }
	;

expr 
	: number_expr { $$ = $1; }
	;

number_expr
	: number_expr2 { $$ = $1; }
	| number_expr TOKEN_ADD number_expr2 { $$ = new NBinaryOperator(eADD, $1, $3); }
	| number_expr TOKEN_SUBTRACT number_expr2 { $$ = new NBinaryOperator(eSUBTRACT, $1, $3); }
	;

number_expr2
	: number_expr3 { $$ = $1; }
	| number_expr2 TOKEN_DIVIDE number_expr3 { $$ = new NBinaryOperator(eDIVIDE, $1, $3); }
	| number_expr2 TOKEN_MULTIPLY number_expr3 { $$ = new NBinaryOperator(eMULTIPLY, $1, $3); }
	;

number_expr3
	: number_expr4 { $$ = $1; }
	| number_expr3 TOKEN_RAISE number_expr4 { $$ = new NBinaryOperator(eRAISE, $1, $3); }
	;

number_expr4
	: number { $$ = $1; }
	| TOKEN_LPAREN number_expr TOKEN_RPAREN { $$ = $2; }
	;

number
	: TOKEN_NUMBER { $$ = NPrimitiveNumber::parse(*$1); delete $1; }
	;