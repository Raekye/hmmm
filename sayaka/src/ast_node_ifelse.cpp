#include "ast_node.h"

ASTNodeIfElse::ASTNodeIfElse(ASTNode* cond, ASTNode* if_true, ASTNode* if_false) {
	this->cond = cond;
	this->if_true = if_true;
	this->if_false = if_false;
}

ASTNodeIfElse::~ASTNodeIfElse() {
	delete this->cond;
	delete this->if_true;
	delete this->if_false;
}

ASTNodeIfElse* ASTNodeIfElse::pass_types(CodeGenContext* code_gen_context, ASTType* type) {
	this->cond = this->cond->pass_types(code_gen_context, code_gen_context->ast_types_resolver.int_ty());
	this->if_true = this->if_true->pass_types(code_gen_context, type);
	this->if_false = this->if_false->pass_types(code_gen_context, type);
	this->type = type;
	return this;
}

llvm::Value* ASTNodeIfElse::gen_code(CodeGenContext* code_gen_context) {
	llvm::Value* cond_val = code_gen_context->builder.CreateICmpNE(this->cond->gen_code(code_gen_context), llvm::ConstantInt::get(llvm::Type::getInt32Ty(code_gen_context->llvm_context), 0, true), "ifcond");

	llvm::Function* fn = code_gen_context->builder.GetInsertBlock()->getParent();
	llvm::BasicBlock* if_true_block = llvm::BasicBlock::Create(code_gen_context->llvm_context, "if_true", fn);
	llvm::BasicBlock* if_false_block = llvm::BasicBlock::Create(code_gen_context->llvm_context, "if_false");
	llvm::BasicBlock* merge_block = llvm::BasicBlock::Create(code_gen_context->llvm_context, "if_merge");
	code_gen_context->builder.CreateCondBr(cond_val, if_true_block, if_false_block);

	code_gen_context->builder.SetInsertPoint(if_true_block);
	llvm::Value* if_true_val = this->if_true->gen_code(code_gen_context);
	code_gen_context->builder.CreateBr(merge_block);
	if_true_block = code_gen_context->builder.GetInsertBlock();

	fn->getBasicBlockList().push_back(if_false_block);
	code_gen_context->builder.SetInsertPoint(if_false_block);
	llvm::Value* if_false_val = this->if_false->gen_code(code_gen_context);
	code_gen_context->builder.CreateBr(merge_block);
	if_false_block = code_gen_context->builder.GetInsertBlock();

	fn->getBasicBlockList().push_back(merge_block);
	code_gen_context->builder.SetInsertPoint(merge_block);

	llvm::PHINode* pn = code_gen_context->builder.CreatePHI(this->type->llvm_type, 2, "iftmp");
	pn->addIncoming(if_true_val, if_true_block);
	pn->addIncoming(if_false_val, if_false_block);
	return pn;
}
