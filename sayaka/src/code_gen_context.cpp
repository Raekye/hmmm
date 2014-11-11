#include "code_gen_context.h"
#include <iostream>

CodeGenContext::CodeGenContext(llvm::LLVMContext& llvm_context) : builder(llvm_context), llvm_context(llvm_context) {
	this->module = new llvm::Module("top", this->llvm_context);
}

CodeGenContext::~CodeGenContext() {
	return;
}

void CodeGenContext::push_block(llvm::BasicBlock* block) {
	std::cout << "Pushing block" << std::endl;
	this->blocks.push(block);
	this->builder.SetInsertPoint(this->current_block());
}

void CodeGenContext::pop_block() {
	std::cout << "Popping block" << std::endl;
	this->blocks.pop();
	this->builder.SetInsertPoint(this->current_block());
}

void CodeGenContext::push_scope() {
	std::cout << "Pushing scope" << std::endl;
	this->scope.push();
}

void CodeGenContext::pop_scope() {
	std::cout << "Popping scope" << std::endl;
	this->scope.pop();
}

llvm::BasicBlock* CodeGenContext::current_block() {
	if (this->blocks.size() == 0) {
		return NULL;
	}
	return this->blocks.top();
}

llvm::Type* CodeGenContext::llvm_pointer_ty() {
	if (sizeof(void*) == 8) {
		return llvm::Type::getInt64Ty(this->llvm_context);
	}
	return llvm::Type::getInt32Ty(this->llvm_context);
}
