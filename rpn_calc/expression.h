#ifndef __EXPRESSION_H_
#define __EXPRESSION_H_

typedef enum tagEOperationType {
	eVALUE,
	eMULTIPLY,
	eADD,
	eSUBTRACT,
	eDIVIDE,
} EOperationType;

typedef struct tagSExpression {
	EOperationType type;
	int value;
	struct tagSExpression* left;
	struct tagSExpression* right;
} SExpression;

SExpression* createNumber(int value);

SExpression* createOperation(EOperationType type, SExpression* left, SExpression* right);

void deleteExpression(SExpression* expr);

#endif // __EXPRESSION_H_