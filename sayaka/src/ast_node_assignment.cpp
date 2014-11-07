#include "ast_node.h"

#include <iostream>

ASTNodeAssignment::ASTNodeAssignment(ASTNodeIdentifier* lhs, ASTNode* rhs) {
	this->lhs = lhs;
	this->rhs = rhs;
}

ASTNodeAssignment::~ASTNodeAssignment() {
	delete this->lhs;
	delete this->rhs;
}

ASTNodeAssignment* ASTNodeAssignment::pass_types(CodeGenContext* code_gen_context, ASTType* ignore) {
	this->lhs = this->lhs->pass_types(code_gen_context, NULL);
	this->rhs = this->rhs->pass_types(code_gen_context, this->lhs->type);
	this->type = this->lhs->type;
	return this;
}

llvm::Value* ASTNodeAssignment::gen_code(CodeGenContext* code_gen_context) {
	std::cout << "Generating assignment" << std::endl;
	CodeGenVariable* var = code_gen_context->scope.get(this->lhs->name);
	if (var == NULL) {
		throw std::runtime_error("Undeclared variable");
	}
	llvm::Value* rhs_val = this->rhs->gen_code(code_gen_context);
	code_gen_context->builder.CreateStore(rhs_val, std::get<1>(*var), false);
	return rhs_val;
}
