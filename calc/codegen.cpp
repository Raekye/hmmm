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

CodeGen::CodeGen(llvm::Module* module) {
	this->module = module;
	//this->scope.push();
	//this->blocks.push(llvm::BasicBlock::Create(llvm::getGlobalContext()));
}

void CodeGen::push_block(llvm::BasicBlock* block) {
	this->blocks.push(block);
}

void CodeGen::pop_block() {
	//delete this->blocks.top();
	this->blocks.pop();
}

llvm::BasicBlock* CodeGen::current_block() {
	return this->blocks.top();
}

void CodeGen::generate_code(NExpression* root) {
}

void CodeGen::run_code() {
}

llvm::Value* NPrimitiveNumber::gen_code(CodeGen* code_gen) {
	std::cout << "Generating int..." << std::endl;
	return llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), this->val.i, true);
}

llvm::Value* NBinaryOperator::gen_code(CodeGen* code_gen) {
	std::cout << "Generating binary operator..." << std::endl;
	llvm::Value* l = this->lhs->gen_code(code_gen);
	llvm::Value* r = this->rhs->gen_code(code_gen);
	if (l == NULL || r == NULL) {
		return NULL;
	}
	builder.SetInsertPoint(code_gen->current_block());
	switch (this->op) {
		case eADD:
			return builder.CreateAdd(l, r, "addtmp");
		case eSUBTRACT:
			return builder.CreateSub(l, r, "subtmp");
		case eMULTIPLY:
			return builder.CreateMul(l, r, "multmp");
		case eDIVIDE:
			return builder.CreateSDiv(l, r, "divtmp");
		case eRAISE:
			return builder.CreateAdd(l, r, "powtmp");
	}
	return ErrorV("Invalid binary operator.");
}

llvm::Value* NFunction::gen_code(CodeGen* code_gen) {
	std::vector<llvm::Type*> arg_types;
	llvm::FunctionType* fn_type = llvm::FunctionType::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), arg_types, false);
	llvm::Function* fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, "", code_gen->module);
	
	llvm::BasicBlock* basic_block = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", fn);
	code_gen->push_block(basic_block);
	code_gen->scope.push();
	builder.SetInsertPoint(basic_block);

	llvm::Value* ret_val = this->body->gen_code(code_gen);
	if (ret_val != NULL) {
		builder.CreateRet(ret_val);
		llvm::verifyFunction(*fn);
		code_gen->pop_block();
		code_gen->scope.pop();
		return fn;
	}
	code_gen->pop_block();
	code_gen->scope.pop();
	return ErrorV("Error generating function");
}

llvm::Value* NIdentifier::gen_code(CodeGen* code_gen) {
	llvm::Value* val = code_gen->scope.get(this->name);
	if (val == NULL) {
		std::cout << "Undeclared variable " << this->name << std::endl;
		return NULL;
	}
	//return builder.CreateLoad(val, false, "");
	return new llvm::LoadInst(val, "", false, code_gen->current_block());
}

llvm::Value* NAssignment::gen_code(CodeGen* code_gen) {
	std::cout << "Generating assignment for " << this->lhs->name << "..." << std::endl;
	llvm::Value* val = code_gen->scope.get(this->lhs->name);
	if (val == NULL) {
		std::cout << "Undeclared variable " << this->lhs->name << std::endl;
		return NULL;
	}
	llvm::Value* rhs_val = this->rhs->gen_code(code_gen);
	builder.SetInsertPoint(code_gen->current_block());
	builder.CreateStore(rhs_val, val, false);
	return rhs_val;
}

llvm::Value* NVariableDeclaration::gen_code(CodeGen* code_gen) {
	std::cout << "Generating variable declaration for " << this->var_name->name << "..." << std::endl;
	builder.SetInsertPoint(code_gen->current_block());
	llvm::AllocaInst* alloc = new llvm::AllocaInst(llvm::Type::getInt64Ty(llvm::getGlobalContext()), this->var_name->name.c_str(), code_gen->current_block()); //builder.CreateAlloca(llvm::Type::getInt64Ty(llvm::getGlobalContext()), 0, this->var_name->name.c_str());
	code_gen->scope.put(this->var_name->name, alloc);
	return llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), 0, true);
}

llvm::Value* NBlock::gen_code(CodeGen* code_gen) {
	llvm::Value* last = NULL;
	for (std::vector<NExpression*>::iterator it = this->statements.begin(); it != this->statements.end(); it++) {
		last = (*it)->gen_code(code_gen);
	}
	return last;
}