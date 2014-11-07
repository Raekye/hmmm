#include "ast_node.h"

#include <iostream>

ASTNodeDeclaration::ASTNodeDeclaration(std::string type_name, std::string var_name) {
	this->type_name = type_name;
	this->var_name = var_name;
}

ASTNodeDeclaration::~ASTNodeDeclaration() {
	return;
}

ASTNodeDeclaration* ASTNodeDeclaration::pass_types(CodeGenContext* code_gen_context, ASTType* ignore) {
	this->type = code_gen_context->ast_types_resolver.get(this->type_name);
	code_gen_context->scope.put(this->var_name, new CodeGenVariable(this->type, NULL));
	return this;
}

llvm::Value* ASTNodeDeclaration::gen_code(CodeGenContext* code_gen_context) {
	std::cout << "Generating declaration" << std::endl;
	ASTType* type = code_gen_context->ast_types_resolver.get(this->type_name);
	if (type == NULL) {
		throw std::runtime_error("Unknown type");
	}
	llvm::AllocaInst* alloc = code_gen_context->builder.CreateAlloca(type->llvm_type, NULL, this->var_name);
	code_gen_context->scope.put(this->var_name, new CodeGenVariable(type, alloc));
	return alloc;
}
