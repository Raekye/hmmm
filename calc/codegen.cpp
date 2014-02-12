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

CodeGenContext::CodeGenContext() {
	this->module = new llvm::Module("main", llvm::getGlobalContext());
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