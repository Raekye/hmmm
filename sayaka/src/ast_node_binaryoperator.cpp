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
	if (this->op % 2 == 0) {
		this->type = code_gen_context->ast_types_resolver.bit_ty();
	} else {
		this->type = type;
	}
	return this;
}

llvm::Value* ASTNodeBinaryOperator::gen_code(CodeGenContext* code_gen_context) {
	std::cout << "Generating binary operator with " << this->lhs->type->name << ", " << this->rhs->type->name << std::endl;
	llvm::Value* l = this->lhs->gen_code(code_gen_context);
	llvm::Value* r = this->rhs->gen_code(code_gen_context);
	if (this->lhs->type->is_primitive() && this->rhs->type->is_primitive()) {
		if (this->lhs->type != this->rhs->type) {
			throw std::runtime_error("Binary operands of different types");
		}
		if (this->lhs->type->is_integral()) {
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
				case eEQ:
					return code_gen_context->builder.CreateICmpEQ(l, r, "eqtmp");
				case eNEQ:
					return code_gen_context->builder.CreateICmpNE(l ,r, "neqtmp");
				case eLT:
					return code_gen_context->builder.CreateICmpSLT(l, r, "lttmp");
				case eGT:
					return code_gen_context->builder.CreateICmpSGE(l, r, "gttmp");
				case eLEQ:
					return code_gen_context->builder.CreateICmpSLE(l, r, "leqtmp");
				case eGEQ:
					return code_gen_context->builder.CreateICmpSGE(l, r, "geqtmp");
			}
		} else {
			switch (this->op) {
				case eADD:
					return code_gen_context->builder.CreateFAdd(l, r, "faddtmp");
				case eSUBTRACT:
					return code_gen_context->builder.CreateFSub(l, r, "fsubtmp");
				case eMULTIPLY:
					return code_gen_context->builder.CreateFMul(l, r, "fmultmp");
				case eDIVIDE:
					return code_gen_context->builder.CreateFDiv(l, r, "fdivtmp");
				case ePOW:
					return code_gen_context->builder.CreateFAdd(l, r, "fpowtmp"); // TODO: library function
				case eEQ:
					return code_gen_context->builder.CreateFCmpUEQ(l, r, "feqtmp");
				case eNEQ:
					return code_gen_context->builder.CreateFCmpUNE(l, r, "fneqtmp");
				case eLT:
					return code_gen_context->builder.CreateFCmpULT(l, r, "flttmp");
				case eGT:
					return code_gen_context->builder.CreateFCmpUGT(l, r, "fgttmp");
				case eLEQ:
					return code_gen_context->builder.CreateFCmpULE(l, r, "fleqtmp");
				case eGEQ:
					return code_gen_context->builder.CreateFCmpUGE(l, r, "fgeqtmp");
			}
		}
		throw std::runtime_error("Unknown binary operation");
	}
	throw std::runtime_error("I tried");
}
