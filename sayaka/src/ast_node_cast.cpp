#include "ast_node_cast.h"

#include <iostream>

ASTNodeCast::ASTNodeCast(ASTNode* val, ASTType* target_type) {
	this->val = val;
	this->target_type = target_type;
	this->type = this->target_type;
}

ASTNodeCast::~ASTNodeCast() {
	return;
}

llvm::Value* ASTNodeCast::gen_code(CodeGen* code_gen) {
	std::cout << "Generating cast..." << std::endl;
	code_gen->builder.SetInsertPoint(code_gen->current_block());
	if (this->val->type->is_primitive() && this->target_type->is_primitive()) {
		llvm::Value* llvm_val = this->val->gen_code(code_gen);
		if (this->val->type->is_integral()) {
			if (this->target_type->is_integral()) {
				return code_gen->builder.CreateIntCast(llvm_val, this->target_type->llvm_type, this->val->type->is_signed());
			} else {
				if (this->val->type->is_signed()) {
					return code_gen->builder.CreateSIToFP(llvm_val, this->target_type->llvm_type);
				} else {
					return code_gen->builder.CreateUIToFP(llvm_val, this->target_type->llvm_type);
				}
			}
		} else {
			if (this->target_type->is_floating()) {
				return code_gen->builder.CreateFPCast(llvm_val, this->target_type->llvm_type);
			} else {
				if (this->target_type->is_signed()) {
					return code_gen->builder.CreateFPToSI(llvm_val, this->target_type->llvm_type);
				} else {
					return code_gen->builder.CreateFPToUI(llvm_val, this->target_type->llvm_type);
				}
			}
		}
	}
	std::cout << "Badness!!!" << std::endl;
	return NULL;
}
