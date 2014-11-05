#include "ast_node_declaration.h"

#include <iostream>

ASTNodeDeclaration::ASTNodeDeclaration(ASTType* type, ASTNodeIdentifier* var_name) {
	this->type = type;
	this->var_name = var_name;
	this->var_name->type = this->type;
}

ASTNodeDeclaration::~ASTNodeDeclaration() {
	delete this->var_name;
}

ASTNodeDeclaration* ASTNodeDeclaration::pass_types(ASTType* type, IdentifierScope scope) {
	if (scope.in_top(this->var_name->name)) {
		throw std::runtime_error("hmmm");
	}
	scope.put(this->var_name->name, this->var_name);
	return this;
}

llvm::Value* ASTNodeDeclaration::gen_code(CodeGen* code_gen) {
	if (this->type == NULL || this->type->llvm_type == NULL) {
		std::cout << "Unknown type " << this->type->name << std::endl;
		return NULL;
	}
	std::cout << "Generating variable declaration for " << this->var_name->name << ", type " << this->type->name << "..." << std::endl;
	code_gen->builder.SetInsertPoint(code_gen->current_block());
	llvm::AllocaInst* alloc = new llvm::AllocaInst(this->type->llvm_type, this->var_name->name.c_str(), code_gen->current_block());
	//this->value = alloc;
	//code_gen->scope.put(this->var_name->name, this);
	return alloc;
}
