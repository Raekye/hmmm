#include "node.h"
#include "codegen.h"
#include "parser.hpp"
#include <iostream>

static void Error(const char* str) {
	std::cout << str << std::endl;
}

static llvm::Value* ErrorV(const char* str) {
	Error(str);
	return NULL;
}

static llvm::IRBuilder<> builder(llvm::getGlobalContext());

CodeGenContext::CodeGenContext(llvm::Module* module) {
	this->module = module;
}

void CodeGenContext::generate_code(NExpression* root) {
}

void CodeGenContext::run_code() {
}

llvm::Value* NPrimitiveNumber::gen_code(CodeGenContext* context) {
	std::cout << "Generating int..." << std::endl;
	return llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), this->val.i, true);
}

llvm::Value* NBinaryOperator::gen_code(CodeGenContext* context) {
	std::cout << "Generating binary operator..." << std::endl;
	llvm::Value* l = this->lhs->gen_code(context);
	llvm::Value* r = this->rhs->gen_code(context);
	if (l == NULL || r == NULL) {
		return NULL;
	}

	switch (this->op) {
		case eADD:
			return builder.CreateAdd(l, r, "addtmp");
		case eSUBTRACT:
			return builder.CreateSub(l, r, "subtmp");
		case eMULTIPLY:
			return builder.CreateMul(l, r, "multmp");
		case eDIVIDE:
			return builder.CreateAdd(l, r, "divtmp");
		case eRAISE:
			return builder.CreateAdd(l, r, "powtmp");
	}
	return ErrorV("Invalid binary operator.");
}

llvm::Value* NFunction::gen_code(CodeGenContext* context) {
	std::vector<llvm::Type*> arg_types;
	llvm::FunctionType* fn_type = llvm::FunctionType::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), arg_types, false);
	llvm::Function* fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, "", context->module);
	
	llvm::BasicBlock* basic_block = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", fn);
	builder.SetInsertPoint(basic_block);

	llvm::Value* ret_val = this->body->gen_code(context);
	if (ret_val != NULL) {
		builder.CreateRet(ret_val);
		llvm::verifyFunction(*fn);
		return fn;
	}
	return ErrorV("Error generating function");
}