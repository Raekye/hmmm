#include "expression.h"

#include <cstdlib>

static SExpression* allocateExpression() {
	SExpression* expr = (SExpression*) malloc(sizeof *expr);
	if (expr == NULL) {
		return NULL;
	}
	expr->type = eVALUE;
	expr->value = 0;
	expr->left = NULL;
	expr->right = NULL;
	expr->cdr = NULL;
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
	if (expr->cdr != NULL) {
		for (int i = 0; i < expr->cdr->size(); i++) {
			deleteExpression((*(expr->cdr))[i]);
		}
	}
	free(expr);
}

SExpression* createVector(SExpression* car) {
	SExpression* expr = allocateExpression();
	if (expr == NULL) {
		return NULL;
	}
	std::vector<SExpression*>* vec = new std::vector<SExpression*>();
	if (vec == NULL) {
		return NULL;
	}
	vec->push_back(car);
	expr->type = eCDR;
	expr->cdr = vec;
	return expr;
}

SExpression* appendVector(SExpression* expr, SExpression* vec) {
	if (vec == NULL) {
		return NULL;
	}
	vec->cdr->push_back(expr);
	return vec;
}