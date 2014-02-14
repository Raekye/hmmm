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
	EBinaryOperationType binaryOperationType;
	std::string* str;
}

%left TOKEN_ADD TOKEN_SUBTRACT
%left TOKEN_MULTIPLY TOKEN_DIVIDE
%left TOKEN_RAISE

%token TOKEN_LPAREN TOKEN_RPAREN
%token <token> TOKEN_ADD TOKEN_MULTIPLY TOKEN_DIVIDE TOKEN_SUBTRACT TOKEN_RAISE
%token <str> TOKEN_NUMBER

%type <expr> program expr number
%type <binaryOperationType> binary_operator

%start program

%%

program
	: expr { *expression = $1; }
	;

expr
	: expr binary_operator number { $$ = new NBinaryOperator($2, $1, $3); }
	| number { $$ = $1; }
	;

number
	: TOKEN_NUMBER { $$ = NPrimitiveNumber::parse(*$1); delete $1; }

binary_operator
	: TOKEN_ADD { $$ = eADD; }
	| TOKEN_SUBTRACT { $$ = eSUBTRACT; }
	| TOKEN_MULTIPLY { $$ = eMULTIPLY; }
	| TOKEN_DIVIDE { $$ = eDIVIDE; }
	| TOKEN_RAISE { $$ = eRAISE; }
	;