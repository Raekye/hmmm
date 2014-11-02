#include "codegen.h"

CodeGen::CodeGen() : builder(llvm::getGlobalContext()) {
	this->module = new llvm::Module("top", llvm::getGlobalContext());
	this->execution_engine = llvm::EngineBuilder(this->module).setErrorStr(&this->execution_engine_error_str).setEngineKind(llvm::EngineKind::JIT).create();
	this->push_block(llvm::BasicBlock::Create(llvm::getGlobalContext()));
}

CodeGen::~CodeGen() {
	delete this->module;
}

void CodeGen::push_block(llvm::BasicBlock* block) {
	this->blocks.push(block);
	this->scope.push();
}

void CodeGen::pop_block() {
	this->blocks.pop();
	this->scope.pop();
}

llvm::BasicBlock* CodeGen::current_block() {
	return this->blocks.top();
}

llvm::Type* CodeGen::llvm_pointer_ty() {
	static llvm::Type* ty = NULL;
	if (ty == NULL) {
		if (sizeof(void*) == 8) {
			ty = llvm::Type::getInt64Ty(llvm::getGlobalContext());
		} else {
			ty = llvm::Type::getInt32Ty(llvm::getGlobalContext());
		}
	}
	return ty;
}
