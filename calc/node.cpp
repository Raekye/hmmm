#include "node.h"
#include <boost/lexical_cast.hpp>
#include <cfloat>
#include <iostream>

Node::~Node() {
	return;
}

NPrimitiveNumber::~NPrimitiveNumber() {
	return;
}

NPrimitiveNumber::NPrimitiveNumber(std::string str/*UNumberValue val, ENumberType type*/) {
	// this->val = val;
	// this->type = type;
	this->str = str;
	// double d = boost::lexical_cast<double>(this->str);
	double d = atof(this->str.c_str());
	if (this->str.find(".") != std::string::npos || this->str.find("E") != std::string::npos || this->str.find("E+") != std::string::npos || this->str.find("E-") != std::string::npos) {
		if (FLT_MIN <= d && d <= FLT_MAX) {
			this->type = eFLOAT;
			this->val.f = (float) d;
		} else {
			this->type = eDOUBLE;
			this->val.d = d;
		}
		return;
	}
	if (-1 << 7 <= d && d < 1 << 7) {
		this->type = eBYTE;
		this->val.b = (int8_t) d;
	} else if (0 <= d && d  < 1 << 8) {
		this->type = eUBYTE;
		this->val.ub = (uint8_t) d;
	} else if (-1 << 15 <= d && d  < 1 << 15) {
		this->type = eSHORT;
		this->val.s = (int16_t) d;
	} else if (0 <= d && d  < 1 << 16) {
		this->type = eUSHORT;
		this->val.us = (uint16_t) d;
	} else if (-1 << 31 <= d && d  < 1 << 31) {
		this->type = eINT;
		this->val.i = (int32_t) d;
	} else if (0 <= d && d  < 1UL << 32) {
		this->type = eUINT;
		this->val.ui = (uint32_t) d;
	} else if (-1LL << 63 <= d && d  < 1LL << 63) {
		this->type = eLONG;
		this->val.l = (int64_t) d;
	} else if (0 <= d && d  < (1ULL << 32) + ((1ULL << 32) - 1)) {
		this->type = eULONG;
		this->val.ul = (uint64_t) d;
	} else if (FLT_MIN <= d && d  <= FLT_MAX) {
		this->type = eFLOAT;
		this->val.f = (float) d;
	} else if (DBL_MIN < d && d <= DBL_MAX) {
		this->type = eDOUBLE;
		this->val.d = (double) d;
	} else {
		std::cout << "Error parsing number." << std::endl;
		this->type = eINT;
		this->val.i = 0;
	}
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
	return new NPrimitiveNumber(str);
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
	return new NPrimitiveNumber(""/*val, type*/);
}
void NPrimitiveNumber_destruct(NPrimitiveNumber* num) {
	num->~NPrimitiveNumber();
}