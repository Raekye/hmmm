%{

#include "node.h"
#include "parser.h"
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
	NBlock* block;
	NIdentifier* identifier;
	std::string* str;
}

%left TOKEN_ADD TOKEN_SUBTRACT TOKEN_MULTIPLY TOKEN_DIVIDE TOKEN_POW

%token TOKEN_LPAREN TOKEN_RPAREN TOKEN_SEMICOLON TOKEN_EQUALS
%token <token> TOKEN_ADD TOKEN_MULTIPLY TOKEN_DIVIDE TOKEN_SUBTRACT TOKEN_POW
%token <str> TOKEN_NUMBER TOKEN_IDENTIFIER

%type <expr> program expr number_expr number_expr2 number_expr3 number_expr4 number assignment_expr variable_declaration_expr
%type <block> stmts
%type <identifier> identifier

%start program

%%

program
	: stmts { *expression = $1; }
	;

stmts
	: expr TOKEN_SEMICOLON { $$ = new NBlock(); $$->statements.push_back($1); }
	| stmts expr TOKEN_SEMICOLON { $$->statements.push_back($2); }

expr 
	: number_expr { $$ = $1; }
	| assignment_expr { $$ = $1; }
	| variable_declaration_expr { $$ = $1; }
	;

identifier
	: TOKEN_IDENTIFIER { $$ = new NIdentifier(*$1); delete $1; }
	;

assignment_expr
	: identifier TOKEN_EQUALS expr { $$ = new NAssignment($1, $3); }
	;

variable_declaration_expr
	: identifier identifier { $$ = new NVariableDeclaration($1, $2); }
	| identifier identifier TOKEN_EQUALS expr { NBlock* block = new NBlock(); block->statements.push_back(new NVariableDeclaration($1, $2)); block->statements.push_back(new NAssignment($2, $4)); $$ = block; }
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
	| number_expr3 TOKEN_POW number_expr4 { $$ = new NBinaryOperator(ePOW, $1, $3); }
	;

number_expr4
	: number { $$ = $1; }
	| identifier { $$ = $1; }
	| TOKEN_LPAREN number_expr TOKEN_RPAREN { $$ = $2; }
	;

number
	: TOKEN_NUMBER { $$ = new NPrimitiveNumber(*$1); delete $1; }
	;
