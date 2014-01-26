#include "expression.h"
#include "parser.h"
#include "lexer.h"

#include <cstdlib>

int yyparse(SExpression** expr, yyscan_t scanner);

int evaluate(SExpression* expr);

int add(int x, int y) {
	return x + y;
}

int subtract(int x, int y) {
	return x - y;
}

int multiply(int x, int y) {
	return x * y;
}

int divide(int x, int y) {
	return x / y;
}

int map(int (*fn)(int, int), SExpression* initial, std::vector<SExpression*>* vec) {
	int accumulator = evaluate(initial);
	for (int i = 0; i < vec->size(); i++) {
		accumulator = fn(accumulator, evaluate((*vec)[i]));
	}
	return accumulator;
}

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
			return map(&multiply, expr->left, expr->right->cdr);
		case eADD:
			return map(&add, expr->left, expr->right->cdr);
		case eSUBTRACT:
			return map(&subtract, expr->left, expr->right->cdr);
		case eDIVIDE:
			return map(&divide, expr->left, expr->right->cdr);
		default:
			return 0;
	}
}

int main() {
	SExpression* expr = NULL;
	//char test[] = " (4 + 2*10 + 3*( 5 + 1 )) / (4 - 2) + 21";
	char test[] = " + 4 5 (- 7 5) (* 3 4) (/ 38 2)";
	//char test[] = "+ 4 5";
	expr = getAST(test);
	int result = evaluate(expr);
	printf("Result of '%s' is %d\n", test, result);
	deleteExpression(expr);
	return 0;
}