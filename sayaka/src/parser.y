%{

#include <map>
#include <deque>
#include <iostream>
#include "parser.h"
#include "lexer.h"
#include "ast_type.h"
#include "ast_node.h"
#include "ast_node_identifier.h"
#include "ast_node_block.h"
#include "ast_node_assignment.h"
#include "ast_node_binaryoperator.h"
#include "ast_node_cast.h"
#include "ast_node_declaration.h"
#include "ast_node_function.h"
#include "ast_node_primitive.h"

void yyerror(YYLTYPE* llocp, ASTNode**, yyscan_t scanner, const char *s) {
	std::cerr << "YYERROR: " << s << std::endl;
}

static std::deque<std::map<std::string, ASTType*>*> identifier_stack;

static void identifier_stack_push() {
	identifier_stack.push_back(new std::map<std::string, ASTType*>);
}
static void identifier_stack_pop() {
	delete identifier_stack.back();
	identifier_stack.pop_back();
}

static ASTType* identifier_stack_get(std::string name) {
	for (std::deque<std::map<std::string, ASTType*>*>::reverse_iterator it = identifier_stack.rbegin(); it != identifier_stack.rend(); it++) {
		std::map<std::string, ASTType*>::iterator found = (*it)->find(name);
		if (found != (*it)->end()) {
			return found->second;
		}
	}
	std::cout << "Undeclared identifier " << name << std::endl;
	throw new std::runtime_error("Undeclared identifier!");
}

static void identifier_stack_put(std::string name, ASTType* type) {
	(*identifier_stack.back())[name] = type;
}

%}

%code requires {

class ASTNode;
class ASTNodeBlock;
class ASTNodeIdentifier;
class ASTType;

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

}

%define api.pure full
/*%define parse.error verbose*/
%error-verbose
%locations
%lex-param { yyscan_t yyscanner }
%parse-param { ASTNode** root }
%parse-param { yyscan_t yyscanner }

%union {
	ASTNode* node;
	ASTNodeBlock* block;
	ASTNodeIdentifier* identifier;
	std::string* str;
	ASTType* type;
}

%left TOKEN_ADD TOKEN_SUBTRACT
%left TOKEN_MULTIPLY TOKEN_DIVIDE
%left TOKEN_POW
%left TOKEN_EQUALS TOKEN_RPAREN

%token TOKEN_LPAREN TOKEN_RPAREN TOKEN_SEMICOLON TOKEN_EQUALS TOKEN_LBRACE TOKEN_RBRACE TOKEN_LBRACKET TOKEN_RBRACKET
%token TOKEN_ADD TOKEN_MULTIPLY TOKEN_DIVIDE TOKEN_SUBTRACT TOKEN_POW
%token <str> TOKEN_NUMBER TOKEN_IDENTIFIER TOKEN_TYPE_NAME

%type <node> program expr number binary_operator_expr assignment_expr variable_declaration_expr cast_expr
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
	: program_start stmts program_end { *root = $2; }
	;

stmts
	: expr TOKEN_SEMICOLON { $$ = new ASTNodeBlock(); $$->push($1); }
	| stmts expr TOKEN_SEMICOLON { $$->push($2); }
	;

expr
	: TOKEN_LPAREN expr TOKEN_RPAREN { $$ = $2; }
	| number { $$ = $1; }
	| identifier { $$ = $1; }
	| variable_declaration_expr { $$ = $1; }
	| assignment_expr { $$ = $1; }
	| cast_expr { $$ = $1; }
	| binary_operator_expr { $$ = $1; }
	;

identifier
	: TOKEN_IDENTIFIER { $$ = new ASTNodeIdentifier(*$1, identifier_stack_get(*$1)); delete $1; }
	;

new_identifier
	: TOKEN_IDENTIFIER { $$ = new ASTNodeIdentifier(*$1, NULL); delete $1; }
	;

type_name
	: TOKEN_TYPE_NAME { $$ = ASTType::get(*$1); delete $1; }
	;

number
	: TOKEN_NUMBER { $$ = new ASTNodePrimitive(*$1); delete $1; }
	;

assignment_expr
	: identifier TOKEN_EQUALS expr { $$ = new ASTNodeAssignment($1, $3); }
	;

variable_declaration_expr
	: type_name new_identifier { identifier_stack_put($2->name, $1); $$ = new ASTNodeDeclaration($1, $2); }
	;

cast_expr
	: TOKEN_LPAREN type_name TOKEN_RPAREN expr { $$ = new ASTNodeCast($4, $2); }
	;

binary_operator_expr
	: expr TOKEN_ADD expr { $$ = new ASTNodeBinaryOperator(eADD, $1, $3); }
	| expr TOKEN_SUBTRACT expr { $$ = new ASTNodeBinaryOperator(eSUBTRACT, $1, $3); }
	| expr TOKEN_DIVIDE expr { $$ = new ASTNodeBinaryOperator(eDIVIDE, $1, $3); }
	| expr TOKEN_MULTIPLY expr { $$ = new ASTNodeBinaryOperator(eMULTIPLY, $1, $3); }
	| expr TOKEN_POW expr { $$ = new ASTNodeBinaryOperator(ePOW, $1, $3); }
	;
