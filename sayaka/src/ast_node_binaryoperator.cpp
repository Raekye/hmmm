#include "ast_node.h"

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

ASTNodeBinaryOperator* ASTNodeBinaryOperator::pass_types(CodeGenContext* code_gen_context, ASTType* type) {
	this->lhs = this->lhs->pass_types(code_gen_context, type);
	this->rhs = this->rhs->pass_types(code_gen_context, type);
	this->type = type;
	return this;
}

llvm::Value* ASTNodeBinaryOperator::gen_code(CodeGenContext* code_gen_context) {
	std::cout << "Generating binary operator" << std::endl;
	llvm::Value* l = this->lhs->gen_code(code_gen_context);
	llvm::Value* r = this->rhs->gen_code(code_gen_context);
	if (this->lhs->type->is_primitive() && this->rhs->type->is_primitive()) {
		switch (this->op) {
			case eADD:
				return code_gen_context->builder.CreateAdd(l, r, "addtmp");
			case eSUBTRACT:
				return code_gen_context->builder.CreateSub(l, r, "subtmp");
			case eMULTIPLY:
				return code_gen_context->builder.CreateMul(l, r, "multmp");
			case eDIVIDE:
				return code_gen_context->builder.CreateSDiv(l, r, "divtmp");
			case ePOW:
				return code_gen_context->builder.CreateAdd(l, r, "powtmp"); // TODO: library function
		}
	}
	throw std::runtime_error("Unknown binary operation");
}
