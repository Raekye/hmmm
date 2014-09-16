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

NPrimitiveNumber::~NPrimitiveNumber() {
	return;
}

NPrimitiveNumber::NPrimitiveNumber(std::string str) {
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

NCast::NCast(NExpression* val, NType* target_type) {
	this->val = val;
	this->target_type = target_type;
	this->type = this->target_type;
}

NCast::~NCast() {
}

NType::NType(std::string name, std::vector<NType*> extends = std::vector<NType*>(), std::vector<NType*> implements = std::vector<NType*>(), llvm::Type* llvm_type = NULL) {
	if (NType::get(name) != NULL) {
		// TODO: badness
	}
	this->name = name;
	this->extends = extends;
	this->implements = implements;
	NType::types[this->name] = this;
}

bool NType::is_primitive() {
	static std::vector<NType*> primitives { NType::byte_ty(), NType::ubyte_ty(), NType::short_ty(), NType::ushort_ty(), NType::int_ty(), NType::uint_ty(), NType::long_ty(), NType::ulong_ty(), NType::float_ty(), NType::double_ty() };
	return std::find(primitives.begin(), primitives.end(), this) != primitives.end();
}

bool NType::is_signed() {
	if (!this->is_primitive()) {
		throw std::logic_error("Member function is_signed called on non primitive type");
	}
	return this->name[0] != 'U';
}

bool NType::is_integral() {
	if (!this->is_primitive()) {
		throw std::logic_error("Member function is_integral called on non primitive type");
	}
	return !this->is_floating();
}

bool NType::is_floating() {
	if (!this->is_primitive()) {
		throw std::logic_error("Member function is_floating called on non primitive type");
	}
	return this == NType::float_ty() || this == NType::double_ty();
}

NType* NType::byte_ty() {
	static NType instance("Byte");
	instance.llvm_type = llvm::Type::getInt8Ty(llvm::getGlobalContext());
	return &instance;
}

NType* NType::ubyte_ty() {
	static NType instance("UByte");
	instance.llvm_type = llvm::Type::getInt8Ty(llvm::getGlobalContext());
	return &instance;
}

NType* NType::short_ty() {
	static NType instance("Short");
	instance.llvm_type = llvm::Type::getInt16Ty(llvm::getGlobalContext());
	return &instance;
}

NType* NType::ushort_ty() {
	static NType instance("UShort");
	instance.llvm_type = llvm::Type::getInt16Ty(llvm::getGlobalContext());
	return &instance;
}

NType* NType::int_ty() {
	static NType instance("Int");
	instance.llvm_type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
	return &instance;
}

NType* NType::uint_ty() {
	static NType instance("UInt");
	instance.llvm_type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
	return &instance;
}

NType* NType::long_ty() {
	static NType instance("Long");
	instance.llvm_type = llvm::Type::getInt64Ty(llvm::getGlobalContext());
	return &instance;
}

NType* NType::ulong_ty() {
	static NType instance("ULong");
	instance.llvm_type = llvm::Type::getInt64Ty(llvm::getGlobalContext());
	return &instance;
}

NType* NType::float_ty() {
	static NType instance("Float");
	instance.llvm_type = llvm::Type::getFloatTy(llvm::getGlobalContext());
	return &instance;
}

NType* NType::double_ty() {
	static NType instance("Double");
	instance.llvm_type = llvm::Type::getDoubleTy(llvm::getGlobalContext());
	return &instance;
}

NType* NType::get(std::string name) {
	std::map<std::string, NType*>::iterator it = NType::types.find(name);
	if (it != NType::types.end()) {
		return it->second;
	}
	return NULL;
}

int NType::__STATIC_INITIALIZER() {
	NType::byte_ty();
	NType::ubyte_ty();
	NType::short_ty();
	NType::ushort_ty();
	NType::int_ty();
	NType::uint_ty();
	NType::long_ty();
	NType::ulong_ty();
	NType::float_ty();
	NType::double_ty();
	return 0;
}

std::map<std::string, NType*> NType::types;
int NType::__STATIC_INIT = NType::__STATIC_INITIALIZER();
