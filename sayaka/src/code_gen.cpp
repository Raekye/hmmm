#include "code_gen.h"
#include <iostream>

CodeGen::CodeGen(llvm::LLVMContext& context, llvm::Module* module) : context(context), builder(context), ast_types_resolver(context) {
	this->module = module;
}

CodeGen::~CodeGen() {
	return;
}

void CodeGen::push_block(llvm::BasicBlock* block) {
	std::cout << "Pushing block" << std::endl;
	this->blocks.push(block);
	this->builder.SetInsertPoint(this->current_block());
}

void CodeGen::pop_block() {
	std::cout << "Popping block" << std::endl;
	this->blocks.pop();
	this->builder.SetInsertPoint(this->current_block());
}

void CodeGen::push_scope() {
	std::cout << "Pushing scope" << std::endl;
	this->scope.push();
}

void CodeGen::pop_scope() {
	std::cout << "Popping scope" << std::endl;
	this->scope.pop();
}

llvm::BasicBlock* CodeGen::current_block() {
	if (this->blocks.size() == 0) {
		return NULL;
	}
	return this->blocks.top();
}

void CodeGen::visit(ASTNodeIdentifier* node) {
	return;
}

void CodeGen::visit(ASTNodePrimitive* node) {
	return;
}

void CodeGen::visit(ASTNodeDeclaration* node) {
	return;
}

void CodeGen::visit(ASTNodeBlock* node) {
	return;
}

void CodeGen::visit(ASTNodeCast* node) {
	return;
}

void CodeGen::visit(ASTNodeAssignment* node) {
	return;
}

void CodeGen::visit(ASTNodeBinaryOperator* node) {
	return;
}

void CodeGen::visit(ASTNodeFunctionPrototype* node) {
	return;
}

void CodeGen::visit(ASTNodeFunction* node) {
	return;
}

void CodeGen::visit(ASTNodeFunctionCall* node) {
	return;
}

void CodeGen::visit(ASTNodeIfElse* node) {
	return;
}
