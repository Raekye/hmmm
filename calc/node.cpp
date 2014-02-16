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
	val.i = atoi(str.c_str());
	return new NPrimitiveNumber(val, eINT);
}

NFunction::NFunction(NExpression* body) {
	this->body = body;
}

NFunction::~NFunction() {
	delete this->body;
}

NIdentifier::NIdentifier(std::string name) {
	this->name = name;
}

NAssignment::NAssignment(NIdentifier* lhs, NExpression* rhs) {
	this->lhs = lhs;
	this->rhs = rhs;
}

NAssignment::~NAssignment() {
	delete this->lhs;
	delete this->rhs;
}

NVariableDeclaration::NVariableDeclaration(NIdentifier* type, NIdentifier* var_name) {
	this->type = type;
	this->var_name = var_name;
}

NVariableDeclaration::~NVariableDeclaration() {
	delete this->type;
	delete this->var_name;
}

NBlock::NBlock() {
	return;
}

NPrimitiveNumber* NPrimitiveNumber_construct(UNumberValue val, ENumberType type) {
	return new NPrimitiveNumber(val, type);
}
void NPrimitiveNumber_destruct(NPrimitiveNumber* num) {
	num->~NPrimitiveNumber();
}