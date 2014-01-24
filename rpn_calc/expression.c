#include "expression.h"

#include <stdlib.h>

static SExpression* allocateExpression() {
	SExpression* expr = (SExpression*) malloc(sizeof *expr);
	if (expr == NULL) {
		return NULL;
	}
	expr->type = eVALUE;
	expr->value = 0;
	expr->left = NULL;
	expr->right = NULL;
	return expr;
}

SExpression* createNumber(int value) {
	SExpression* expr = allocateExpression();
	if (expr == NULL) {
		return NULL;
	}
	expr->type = eVALUE;
	expr->value = value;
	return expr;
}

SExpression* createOperation(EOperationType type, SExpression* left, SExpression* right) {
	SExpression* expr = allocateExpression();
	if (expr == NULL) {
		return NULL;
	}
	expr->type = type;
	expr->left = left;
	expr->right = right;
	return expr;
}

void deleteExpression(SExpression* expr) {
	if (expr == NULL) {
		return;
	}
	deleteExpression(expr->left);
	deleteExpression(expr->right);
	free(expr);
}