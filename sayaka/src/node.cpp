#include "node.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <cfloat>
#include <iostream>
#include "codegen.h"
#include <algorithm>

Node::~Node() {
	return;
}

NExpression::~NExpression() {
	return;
}

NPrimitive::~NPrimitive() {
	return;
}

NPrimitive::NPrimitive(std::string str) {
	this->str = str;
	boost::replace_all(str, "_", "");
	boost::algorithm::to_upper(str);
	this->type = NType::long_ty();
}

NBinaryOperator::NBinaryOperator(EBinaryOperationType op, NExpression* lhs, NExpression* rhs) {
	this->op = op;
	this->lhs = lhs;
	this->rhs = rhs;
	this->type = this->lhs->type;
}

NBinaryOperator::~NBinaryOperator() {
	delete this->lhs;
	delete this->rhs;
}

NFunction::NFunction(NExpression* body, NType* return_type) {
	this->body = body;
	this->return_type = return_type;
}

NFunction::~NFunction() {
	delete this->body;
}

NIdentifier::NIdentifier(std::string name, NType* type) {
	this->name = name;
	this->type = type;
}

NIdentifier::~NIdentifier() {
	return;
}

NAssignment::NAssignment(NIdentifier* lhs, NExpression* rhs) {
	this->lhs = lhs;
	this->rhs = rhs;
	this->type = lhs->type;
}

NAssignment::~NAssignment() {
	delete this->lhs;
	delete this->rhs;
}

NVariableDeclaration::NVariableDeclaration(NType* type, NIdentifier* var_name) {
	this->type = type;
	this->var_name = var_name;
	this->var_name->type = this->type;
}

NVariableDeclaration::~NVariableDeclaration() {
	delete this->type;
	delete this->var_name;
}

NBlock::NBlock() {
	return;
}

void NBlock::push(NExpression* expr) {
	this->type = expr->type;
	this->statements.push_back(expr);
}

NBlock::~NBlock() {
	return;
}

NCast::NCast(NExpression* val, NType* target_type) {
	this->val = val;
	this->target_type = target_type;
	this->type = this->target_type;
}

NCast::~NCast() {
}
