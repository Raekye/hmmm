#include "node.h"

Node::~Node() {
	return;
}

NPrimitiveNumber::~NPrimitiveNumber() {
	return;
}

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

NFunction::NFunction(NExpression* body) {
	this->body = body;
}

NFunction::~NFunction() {
	delete this->body;
}

NPrimitiveNumber* NPrimitiveNumber_construct(UNumberValue val, ENumberType type) {
	return new NPrimitiveNumber(val, type);
}
void NPrimitiveNumber_destruct(NPrimitiveNumber* num) {
	num->~NPrimitiveNumber();
}