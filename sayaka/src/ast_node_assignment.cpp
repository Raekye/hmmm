#include "ast_node_assignment.h"

#include <iostream>

ASTNodeAssignment::ASTNodeAssignment(ASTNodeIdentifier* lhs, ASTNode* rhs) {
	this->lhs = lhs;
	this->rhs = rhs;
	this->type = lhs->type;
}

ASTNodeAssignment::~ASTNodeAssignment() {
	delete this->lhs;
	delete this->rhs;
}

llvm::Value* ASTNodeAssignment::gen_code(CodeGen* code_gen) {
	std::cout << "Generating assignment to " << this->lhs->name << "..." << std::endl;
	ASTNode* val = code_gen->scope.get(this->lhs->name);
	if (val == NULL) {
		std::cout << "Undeclared variable " << this->lhs->name << std::endl;
		return NULL;
	}
	this->type = val->type;
	this->rhs->type = val->type;
	llvm::Value* rhs_val = this->rhs->gen_code(code_gen);
	code_gen->builder.SetInsertPoint(code_gen->current_block());
	code_gen->builder.CreateStore(rhs_val, val->value, false);
	return rhs_val;
}