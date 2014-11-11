%{

#include <map>
#include <deque>
#include <iostream>
#include "parser.h"
#include "lexer.h"
#include "ast_node.h"

void yyerror(YYLTYPE* llocp, ASTNode**, yyscan_t scanner, const char *s) {
	std::cerr << "YYERROR: " << s << std::endl;
}

%}

%code requires {

#include <vector>
#include <tuple>

class ASTNode;
class ASTNodeBlock;
class ASTNodeIdentifier;
class ASTNodeDeclaration;

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
	std::vector<ASTNodeDeclaration*>* typed_args_list;
	std::vector<ASTNode*>* args_list;
	std::string* str;
}

%left TOKEN_ADD TOKEN_SUBTRACT
%left TOKEN_MULTIPLY TOKEN_DIVIDE
%left TOKEN_POW
%left TOKEN_EQUALS TOKEN_RPAREN

%token TOKEN_LPAREN TOKEN_RPAREN TOKEN_SEMICOLON TOKEN_EQUALS TOKEN_LBRACE TOKEN_RBRACE TOKEN_LBRACKET TOKEN_RBRACKET
%token TOKEN_ADD TOKEN_MULTIPLY TOKEN_DIVIDE TOKEN_SUBTRACT TOKEN_POW
%token TOKEN_COMMA
%token <str> TOKEN_NUMBER TOKEN_IDENTIFIER TOKEN_TYPE_NAME

%type <node> program expr number binary_operator_expr assignment_expr variable_declaration_expr cast_expr function_call_expr function_prototype_expr function_expr
%type <block> stmts
%type <identifier> identifier
%type <typed_args_list> typed_args_list
%type <args_list> args_list

%start program

%%

program
	: stmts { *root = $1; }
	;

stmts
	: expr TOKEN_SEMICOLON {
		$$ = new ASTNodeBlock();
		$$->push($1);
	}
	| stmts expr TOKEN_SEMICOLON { $$->push($2); }
	;

expr
	: TOKEN_LPAREN expr TOKEN_RPAREN { $$ = $2; }
	| number { $$ = $1; }
	| identifier { $$ = $1; }
	| variable_declaration_expr { $$ = $1; }
	| assignment_expr { $$ = $1; }
	| cast_expr { $$ = $1; }
	| function_expr { $$ = $1; }
	| function_prototype_expr { $$ = $1; }
	| function_call_expr { $$ = $1; }
	| binary_operator_expr { $$ = $1; }
	;

identifier
	: TOKEN_IDENTIFIER {
		$$ = new ASTNodeIdentifier(*$1);
		delete $1;
	}
	;

number
	: TOKEN_NUMBER {
		$$ = new ASTNodePrimitive(*$1);
		delete $1;
	}
	;

assignment_expr
	: identifier TOKEN_EQUALS expr { $$ = new ASTNodeAssignment($1, $3); }
	;

variable_declaration_expr
	: TOKEN_TYPE_NAME TOKEN_IDENTIFIER {
		$$ = new ASTNodeDeclaration(*$1, *$2);
		delete $1;
		delete $2;
	}
	;

cast_expr
	: TOKEN_LPAREN TOKEN_TYPE_NAME TOKEN_RPAREN expr {
		$$ = new ASTNodeCast(*$2, $4);
		delete $2;
	}
	;

binary_operator_expr
	: expr TOKEN_ADD expr { $$ = new ASTNodeBinaryOperator(eADD, $1, $3); }
	| expr TOKEN_SUBTRACT expr { $$ = new ASTNodeBinaryOperator(eSUBTRACT, $1, $3); }
	| expr TOKEN_DIVIDE expr { $$ = new ASTNodeBinaryOperator(eDIVIDE, $1, $3); }
	| expr TOKEN_MULTIPLY expr { $$ = new ASTNodeBinaryOperator(eMULTIPLY, $1, $3); }
	| expr TOKEN_POW expr { $$ = new ASTNodeBinaryOperator(ePOW, $1, $3); }
	;

function_prototype_expr
	: TOKEN_TYPE_NAME TOKEN_IDENTIFIER TOKEN_LPAREN typed_args_list TOKEN_RPAREN {
		$$ = new ASTNodeFunctionPrototype(*$1, *$2, $4);
		delete $1;
		delete $2;
	}
	;

function_expr
	: function_prototype_expr TOKEN_LBRACE stmts TOKEN_RBRACE {
		$$ = new ASTNodeFunction((ASTNodeFunctionPrototype*) $1, $3);
	}
	;

typed_args_list
	: variable_declaration_expr {
		$$ = new std::vector<ASTNodeDeclaration*>();
		$$->push_back((ASTNodeDeclaration*) $1);
	}
	| typed_args_list TOKEN_COMMA variable_declaration_expr {
		$$->push_back((ASTNodeDeclaration*) $3);
	}
	;

args_list
	: expr {
		$$ = new std::vector<ASTNode*>();
		$$->push_back($1);
	}
	| args_list TOKEN_COMMA expr {
		$$->push_back($3);
	}
	;

function_call_expr
	: TOKEN_IDENTIFIER TOKEN_LPAREN args_list TOKEN_RPAREN {
		$$ = new ASTNodeFunctionCall(*$1, $3);
		delete $1;
	}
	;
