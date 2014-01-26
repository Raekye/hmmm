#ifndef __EXPRESSION_H_
#define __EXPRESSION_H_

#include <vector>

typedef enum tagEOperationType {
	eVALUE,
	eMULTIPLY,
	eADD,
	eSUBTRACT,
	eDIVIDE,
	eCDR,
} EOperationType;

typedef struct tagSExpression {
	EOperationType type;
	int value;
	struct tagSExpression* left;
	struct tagSExpression* right;
	std::vector<struct tagSExpression*>* cdr;
} SExpression;

SExpression* createNumber(int value);

SExpression* createOperation(EOperationType type, SExpression* left, SExpression* right);

void deleteExpression(SExpression* expr);

SExpression* createVector(SExpression* expr);

SExpression* appendVector(SExpression* expr, SExpression* vec);

#endif // __EXPRESSION_H_