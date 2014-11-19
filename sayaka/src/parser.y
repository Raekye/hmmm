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

class ASTNode;
class ASTNodeBlock;
class ASTNodeIdentifier;
class ASTNodeDeclaration;
class ASTNodeFunctionPrototype;

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
	ASTNodeFunctionPrototype* function_prototype;
	ASTNodeDeclaration* declaration;
	std::vector<ASTNodeDeclaration*>* typed_args_list;
	std::vector<ASTNode*>* args_list;
	std::string* str;
}

%right TOKEN_ASSIGN
%left TOKEN_EQ TOKEN_NEQ TOKEN_LT TOKEN_GT TOKEN_LEQ TOKEN_GEQ
%left TOKEN_ADD TOKEN_SUBTRACT
%left TOKEN_MULTIPLY TOKEN_DIVIDE
%left TOKEN_POW
%left TOKEN_RPAREN

%token TOKEN_LPAREN TOKEN_RPAREN TOKEN_ASSIGN TOKEN_LBRACE TOKEN_RBRACE TOKEN_LBRACKET TOKEN_RBRACKET
%token TOKEN_ADD TOKEN_MULTIPLY TOKEN_DIVIDE TOKEN_SUBTRACT TOKEN_POW
%token TOKEN_COMMA TOKEN_IF TOKEN_ELSE TOKEN_VAR TOKEN_VAL
%token TOKEN_SEMICOLON TOKEN_COLON
%token <str> TOKEN_NUMBER TOKEN_IDENTIFIER TOKEN_TYPE_NAME

%type <node> program expr number binary_operator_expr assignment_expr cast_expr function_call_expr function_expr if_else_expr
%type <block> stmts
%type <identifier> identifier
%type <function_prototype> function_prototype_expr
%type <declaration> declaration_expr
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
	| declaration_expr { $$ = $1; }
	| assignment_expr { $$ = $1; }
	| cast_expr { $$ = $1; }
	| if_else_expr { $$ = $1; }
	| function_expr { $$ = $1; }
	| function_prototype_expr { $$ = new ASTNodeFunction($1, NULL); }
	| function_call_expr { $$ = $1; }
	| binary_operator_expr { $$ = $1; }
	| declaration_expr TOKEN_ASSIGN expr {
		std::cout << "here" << std::endl;
		ASTNodeBlock* block = new ASTNodeBlock();
		block->push($1);
		block->push(new ASTNodeAssignment(new ASTNodeIdentifier($1->var_name), $3));
		$$ = block;
	}
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
	: identifier TOKEN_ASSIGN expr { $$ = new ASTNodeAssignment($1, $3); }
	;

declaration_expr
	: TOKEN_VAR TOKEN_IDENTIFIER TOKEN_COLON TOKEN_TYPE_NAME {
		$$ = new ASTNodeDeclaration(*$4, *$2);
		delete $2;
		delete $4;
	}
	;

cast_expr
	: TOKEN_LPAREN TOKEN_TYPE_NAME TOKEN_RPAREN expr {
		$$ = new ASTNodeCast(*$2, $4);
		delete $2;
	}
	;

if_else_expr
	: TOKEN_IF TOKEN_LPAREN expr[cond] TOKEN_RPAREN TOKEN_LBRACE stmts[if_true] TOKEN_RBRACE TOKEN_ELSE TOKEN_LBRACE stmts[if_false] TOKEN_RBRACE {
		$$ = new ASTNodeIfElse($cond, $if_true, $if_false);
	}
	;

binary_operator_expr
	: expr TOKEN_ADD expr { $$ = new ASTNodeBinaryOperator(eADD, $1, $3); }
	| expr TOKEN_SUBTRACT expr { $$ = new ASTNodeBinaryOperator(eSUBTRACT, $1, $3); }
	| expr TOKEN_DIVIDE expr { $$ = new ASTNodeBinaryOperator(eDIVIDE, $1, $3); }
	| expr TOKEN_MULTIPLY expr { $$ = new ASTNodeBinaryOperator(eMULTIPLY, $1, $3); }
	| expr TOKEN_POW expr { $$ = new ASTNodeBinaryOperator(ePOW, $1, $3); }
	| expr TOKEN_EQ expr { $$ = new ASTNodeBinaryOperator(eEQ, $1, $3); }
	| expr TOKEN_NEQ expr { $$ = new ASTNodeBinaryOperator(eNEQ, $1, $3); }
	| expr TOKEN_LT expr { $$ = new ASTNodeBinaryOperator(eLT, $1, $3); }
	| expr TOKEN_GT expr { $$ = new ASTNodeBinaryOperator(eGT, $1, $3); }
	| expr TOKEN_LEQ expr { $$ = new ASTNodeBinaryOperator(eLEQ, $1, $3); }
	| expr TOKEN_GEQ expr { $$ = new ASTNodeBinaryOperator(eGEQ, $1, $3); }
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
		$$ = new ASTNodeFunction($1, $3);
	}
	;

function_call_expr
	: TOKEN_IDENTIFIER TOKEN_LPAREN args_list TOKEN_RPAREN {
		$$ = new ASTNodeFunctionCall(*$1, $3);
		delete $1;
	}
	;

typed_args_list
	: declaration_expr {
		$$ = new std::vector<ASTNodeDeclaration*>();
		$$->push_back($1);
	}
	| typed_args_list TOKEN_COMMA declaration_expr {
		$$->push_back($3);
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
