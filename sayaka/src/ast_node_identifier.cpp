#include "ast_node.h"

#include <iostream>
#include <sstream>

ASTNodeIdentifier::ASTNodeIdentifier(std::string name) {
	this->name = name;
}

ASTNodeIdentifier::~ASTNodeIdentifier() {
	return;
}

ASTNodeIdentifier* ASTNodeIdentifier::pass_types(CodeGenContext* code_gen_context, ASTType* ignore) {
	CodeGenVariable* var = code_gen_context->scope.get(this->name);
	if (var == NULL) {
		throw std::runtime_error("Undeclared identifier");
	}
	this->type = std::get<0>(*var);
	return this;
}

llvm::Value* ASTNodeIdentifier::gen_code(CodeGenContext* code_gen_context) {
	std::cout << "Generating identifier " << this->name << std::endl;
	CodeGenVariable* var = code_gen_context->scope.get(this->name);
	if (var == NULL) {
		throw std::runtime_error("Undeclared variable");
	}
	llvm::Value* val = std::get<1>(*var);
	if (llvm::AllocaInst* alloc = dynamic_cast<llvm::AllocaInst*>(val)) {
		return code_gen_context->builder.CreateLoad(alloc, false, this->name);
	}
	return val;
}
