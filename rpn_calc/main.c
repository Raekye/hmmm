#include "expression.h"
#include "parser.h"
#include "lexer.h"

#include <cstdlib>

int yyparse(SExpression** expr, yyscan_t scanner);

SExpression* getAST(const char* str) {
	SExpression* expr = NULL;
	yyscan_t scanner;
	YY_BUFFER_STATE state;
	if (yylex_init(&scanner)) {
		return NULL;
	}
	state = yy_scan_string(str, scanner);
	if (yyparse(&expr, scanner)) {
		return NULL;
	}
	yy_delete_buffer(state, scanner);
	yylex_destroy(scanner);
	return expr;
}

int evaluate(SExpression* expr) {
	switch (expr->type) {
		case eVALUE:
			return expr->value;
		case eMULTIPLY:
			return evaluate(expr->left) * evaluate(expr->right);
		case eADD:
			return evaluate(expr->left) + evaluate(expr->right);
		case eSUBTRACT:
			return evaluate(expr->left) - evaluate(expr->right);
		case eDIVIDE:
			return evaluate(expr->left) / evaluate(expr->right);
		default:
			return 0;
	}
}

int main() {
	SExpression* expr = NULL;
	char test[] = " (4 + 2*10 + 3*( 5 + 1 )) / (4 - 2) + 21";
	expr = getAST(test);
	int result = evaluate(expr);
	printf("Result of '%s' is %d\n", test, result);
	deleteExpression(expr);
	return 0;
}