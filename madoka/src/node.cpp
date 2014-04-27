#include "node.h"
#include <boost/lexical_cast.hpp>
#include <cfloat>
#include <iostream>
#include "codegen.h"

Node::~Node() {
	return;
}

NPrimitiveNumber::~NPrimitiveNumber() {
	return;
}

NPrimitiveNumber::NPrimitiveNumber(std::string str) {
	this->val.l = boost::lexical_cast<int64_t>(str);
	this->type = NType::long_ty();
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

NFunction::NFunction(NExpression* body, NType* return_type) {
	this->body = body;
	this->return_type = return_type;
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

NVariableDeclaration::NVariableDeclaration(NIdentifier* type_name, NIdentifier* var_name) {
	this->type_name = type_name;
	this->type = NType::get(this->type_name->name);
	this->var_name = var_name;
}

NVariableDeclaration::~NVariableDeclaration() {
	delete this->type;
	delete this->var_name;
}

NBlock::NBlock() {
	return;
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

NType* NType::byte_ty() {
	static NType instance("byte");
	instance.llvm_type = llvm::Type::getInt8Ty(llvm::getGlobalContext());
	return &instance;
}

NType* NType::ubyte_ty() {
	static NType instance("ubyte");
	instance.llvm_type = llvm::Type::getInt8Ty(llvm::getGlobalContext());
	return &instance;
}

NType* NType::short_ty() {
	static NType instance("short");
	instance.llvm_type = llvm::Type::getInt16Ty(llvm::getGlobalContext());
	return &instance;
}

NType* NType::ushort_ty() {
	static NType instance("ushort");
	instance.llvm_type = llvm::Type::getInt16Ty(llvm::getGlobalContext());
	return &instance;
}

NType* NType::int_ty() {
	static NType instance("int");
	instance.llvm_type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
	return &instance;
}

NType* NType::uint_ty() {
	static NType instance("uint");
	instance.llvm_type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
	return &instance;
}

NType* NType::long_ty() {
	static NType instance("long");
	instance.llvm_type = llvm::Type::getInt64Ty(llvm::getGlobalContext());
	return &instance;
}

NType* NType::ulong_ty() {
	static NType instance("ulong");
	instance.llvm_type = llvm::Type::getInt64Ty(llvm::getGlobalContext());
	return &instance;
}

NType* NType::float_ty() {
	static NType instance("float");
	instance.llvm_type = llvm::Type::getFloatTy(llvm::getGlobalContext());
	return &instance;
}

NType* NType::double_ty() {
	static NType instance("double");
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
