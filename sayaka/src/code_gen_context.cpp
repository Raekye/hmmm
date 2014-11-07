#include "code_gen_context.h"

CodeGenContext::CodeGenContext(llvm::LLVMContext& llvm_context) : builder(llvm_context), llvm_context(llvm_context) {
	this->module = new llvm::Module("top", this->llvm_context);
	this->push_block();
}

CodeGenContext::~CodeGenContext() {
	//delete this->module;
}

void CodeGenContext::push_block() {
	this->push_block(llvm::BasicBlock::Create(this->llvm_context));
}

void CodeGenContext::push_block(llvm::BasicBlock* block) {
	this->blocks.push(block);
	this->scope.push();
	this->builder.SetInsertPoint(this->current_block());
}

void CodeGenContext::pop_block() {
	this->blocks.pop();
	this->scope.pop();
	this->builder.SetInsertPoint(this->current_block());
}

llvm::BasicBlock* CodeGenContext::current_block() {
	return this->blocks.top();
}

llvm::Type* CodeGenContext::llvm_pointer_ty() {
	if (sizeof(void*) == 8) {
		return llvm::Type::getInt64Ty(this->llvm_context);
	}
	return llvm::Type::getInt32Ty(this->llvm_context);
}
