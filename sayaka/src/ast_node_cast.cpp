#include "ast_node_cast.h"

#include <iostream>

ASTNodeCast::ASTNodeCast(ASTNode* val, ASTType* type) {
	this->val = val;
	this->type = type;
}

ASTNodeCast::~ASTNodeCast() {
	delete this->val;
}

ASTNodeCast* ASTNodeCast::pass_types(ASTType* type, IdentifierScope scope) {
	this->val = this->val->pass_types(this->type, scope);
	return this;
}

llvm::Value* ASTNodeCast::gen_code(CodeGen* code_gen) {
	std::cout << "Generating cast..." << std::endl;
	code_gen->builder.SetInsertPoint(code_gen->current_block());
	if (this->val->type->is_primitive() && this->type->is_primitive()) {
		llvm::Value* llvm_val = this->val->gen_code(code_gen);
		if (this->val->type->is_integral()) {
			if (this->type->is_integral()) {
				return code_gen->builder.CreateIntCast(llvm_val, this->type->llvm_type, this->val->type->is_signed());
			} else {
				if (this->val->type->is_signed()) {
					return code_gen->builder.CreateSIToFP(llvm_val, this->type->llvm_type);
				} else {
					return code_gen->builder.CreateUIToFP(llvm_val, this->type->llvm_type);
				}
			}
		} else {
			if (this->type->is_floating()) {
				return code_gen->builder.CreateFPCast(llvm_val, this->type->llvm_type);
			} else {
				if (this->type->is_signed()) {
					return code_gen->builder.CreateFPToSI(llvm_val, this->type->llvm_type);
				} else {
					return code_gen->builder.CreateFPToUI(llvm_val, this->type->llvm_type);
				}
			}
		}
	}
	throw std::runtime_error("hmmm");
}
