#include "ast_node.h"

#include <iostream>

ASTNodeCast::ASTNodeCast(std::string target_type, ASTNode* val) {
	this->target_type = target_type;
	this->val = val;
}

ASTNodeCast::~ASTNodeCast() {
	delete this->val;
}

ASTNodeCast* ASTNodeCast::pass_types(CodeGenContext* code_gen_context, ASTType* ignore) {
	this->type = code_gen_context->ast_types_resolver.get(this->target_type);
	this->val = this->val->pass_types(code_gen_context, this->type);
	return this;
}

llvm::Value* ASTNodeCast::gen_code(CodeGenContext* code_gen_context) {
	std::cout << "Generating cast" << std::endl;
	if (this->val->type->is_primitive() && this->type->is_primitive()) {
		llvm::Value* llvm_val = this->val->gen_code(code_gen_context);
		if (this->val->type->is_integral()) {
			if (this->type->is_integral()) {
				return code_gen_context->builder.CreateIntCast(llvm_val, this->type->llvm_type, this->val->type->is_signed());
			} else {
				if (this->val->type->is_signed()) {
					return code_gen_context->builder.CreateSIToFP(llvm_val, this->type->llvm_type);
				} else {
					return code_gen_context->builder.CreateUIToFP(llvm_val, this->type->llvm_type);
				}
			}
		} else {
			if (this->type->is_floating()) {
				return code_gen_context->builder.CreateFPCast(llvm_val, this->type->llvm_type);
			} else {
				if (this->type->is_signed()) {
					return code_gen_context->builder.CreateFPToSI(llvm_val, this->type->llvm_type);
				} else {
					return code_gen_context->builder.CreateFPToUI(llvm_val, this->type->llvm_type);
				}
			}
		}
	}
	throw std::runtime_error("hmmm");
}
