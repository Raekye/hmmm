#include "node.h"

NPrimitiveNumber::NPrimitiveNumber(UNumberValue val, ENumberType type) {
	this->val = val;
	this->type = type;
}

NBinaryOperator::NBinaryOperator(EBinaryOperationType op, NExpression* lhs, NExpression* rhs) {
	this->op = op;
	this->lhs = lhs;
	this->rhs = rhs;
}

NBinaryOperator::~NBinaryOperator() {
	delete this->lhs;
	delete this->rhs;
}

NPrimitiveNumber* NPrimitiveNumber::parse(std::string str) {
	UNumberValue val;
	val.i = 32;
	return new NPrimitiveNumber(val, eINT);
}