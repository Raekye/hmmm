#include "node.h"
#include "codegen.h"
#include "parser.hpp"
#include <iostream>

CodeGenContext::CodeGenContext() {
	this->module = new llvm::Module("main", llvm::getGlobalContext());
}

void CodeGenContext::generate_code(NExpression* root) {
}

void CodeGenContext::run_code() {
}

llvm::Value* NPrimitiveNumber::gen_code(CodeGenContext& context) {
}

llvm::Value* NBinaryOperator::gen_code(CodeGenContext& context) {
}