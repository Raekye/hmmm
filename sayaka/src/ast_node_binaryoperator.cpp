#include "ast_node_binaryoperator.h"

#include <iostream>

ASTNodeBinaryOperator::ASTNodeBinaryOperator(EBinaryOperationType op, ASTNode* lhs, ASTNode* rhs) {
	this->op = op;
	this->lhs = lhs;
	this->rhs = rhs;
}

ASTNodeBinaryOperator::~ASTNodeBinaryOperator() {
	delete this->lhs;
	delete this->rhs;
}

ASTNodeBinaryOperator* ASTNodeBinaryOperator::pass_types(ASTType* type, IdentifierScope scope) {
	this->lhs = this->lhs->pass_types(type, scope);
	this->rhs = this->rhs->pass_types(type, scope);
	return this;
}

llvm::Value* ASTNodeBinaryOperator::gen_code(CodeGen* code_gen) {
	std::cout << "Generating binary operator..." << std::endl;
	this->lhs->type = this->type;
	this->rhs->type = this->type;
	llvm::Value* l = this->lhs->gen_code(code_gen);
	llvm::Value* r = this->rhs->gen_code(code_gen);
	if (this->lhs->type->is_primitive() && this->rhs->type->is_primitive()) {
		switch (this->op) {
			case eADD:
				return code_gen->builder.CreateAdd(l, r, "addtmp");
			case eSUBTRACT:
				return code_gen->builder.CreateSub(l, r, "subtmp");
			case eMULTIPLY:
				return code_gen->builder.CreateMul(l, r, "multmp");
			case eDIVIDE:
				return code_gen->builder.CreateSDiv(l, r, "divtmp");
			case ePOW:
				return code_gen->builder.CreateAdd(l, r, "powtmp"); // TODO: library function
		}
	}
	throw std::runtime_error("Unknown binary operation");
}
