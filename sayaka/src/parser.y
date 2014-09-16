%{

#include "node.h"
#include "parser.h"
#include "lexer.h"
#include <cstdio>
#include <map>
#include <deque>
#include <iostream>

// TODO: memory management

void yyerror(NExpression**, yyscan_t scanner, const char *s) {
	fprintf(stderr, "YYERROR: %s!", s);
}

NExpression* programBlock;

static std::deque<std::map<std::string, NType*>*> identifier_stack;

static void identifier_stack_push() {
	identifier_stack.push_back(new std::map<std::string, NType*>);
}
static void identifier_stack_pop() {
	delete identifier_stack.back();
	identifier_stack.pop_back(); // TODO: does this call destructors?
}

static NType* identifier_stack_get(std::string name) {
	for (std::deque<std::map<std::string, NType*>*>::reverse_iterator it = identifier_stack.rbegin(); it != identifier_stack.rend(); it++) {
		std::map<std::string, NType*>::iterator found = (*it)->find(name);
		if (found != (*it)->end()) {
			return found->second;
		}
	}
	std::cout << "Undeclared identifier " << name << std::endl;
	throw new std::runtime_error("Undeclared identifier!");
}

static void identifier_stack_put(std::string name, NType* type) {
	(*identifier_stack.back())[name] = type;
}

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
	NExpression* expr;
	NBlock* block;
	NIdentifier* identifier;
	std::string* str;
	NType* type;
}

%left TOKEN_ADD TOKEN_SUBTRACT TOKEN_MULTIPLY TOKEN_DIVIDE TOKEN_POW

%token TOKEN_LPAREN TOKEN_RPAREN TOKEN_SEMICOLON TOKEN_EQUALS TOKEN_LBRACE TOKEN_RBRACE
%token <token> TOKEN_ADD TOKEN_MULTIPLY TOKEN_DIVIDE TOKEN_SUBTRACT TOKEN_POW
%token <str> TOKEN_NUMBER TOKEN_IDENTIFIER TOKEN_TYPE_NAME

%type <expr> program expr number_expr number_expr2 number_expr3 number_expr4 number assignment_expr variable_declaration_expr cast_expr
%type <block> stmts
%type <identifier> identifier new_identifier
%type <type> type_name

%start program

%%

program_start
	: { identifier_stack_push(); }
	;

program_end
	: { identifier_stack_pop(); }
	;

program
	: program_start stmts program_end { *expression = $2; }
	;

stmts
	: expr TOKEN_SEMICOLON { $$ = new NBlock(); $$->push($1); }
	| stmts expr TOKEN_SEMICOLON { $$->push($2); }

expr
	: cast_expr { $$ = $1; }
	| number_expr { $$ = $1; }
	| assignment_expr { $$ = $1; }
	| variable_declaration_expr { $$ = $1; }
	;

identifier
	: TOKEN_IDENTIFIER { $$ = new NIdentifier(*$1, identifier_stack_get(*$1)); delete $1; }
	;

new_identifier
	: TOKEN_IDENTIFIER { $$ = new NIdentifier(*$1, NULL); delete $1; }
	;

type_name
	: TOKEN_TYPE_NAME { $$ = NType::get(*$1); delete $1; }
	;

number
	: TOKEN_NUMBER { $$ = new NPrimitiveNumber(*$1); delete $1; }
	;

assignment_expr
	: identifier TOKEN_EQUALS expr { $$ = new NAssignment($1, $3); }
	;

variable_declaration_expr
	: type_name new_identifier { identifier_stack_put($2->name, $1); $$ = new NVariableDeclaration($1, $2); }
	;

cast_expr
	: TOKEN_LPAREN type_name TOKEN_RPAREN expr { $$ = new NCast($4, $2); }
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
