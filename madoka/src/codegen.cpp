#include "node.h"
#include "codegen.h"
#include "parser.h"
#include <iostream>
#include <boost/lexical_cast.hpp>
#include "number.h"
#include <sstream>

static llvm::IRBuilder<> builder(llvm::getGlobalContext());

static void error(std::string str) {
	std::cout << str << std::endl;
}

CodeGen::CodeGen(llvm::Module* module) {
	this->module = module;
	this->push_block(llvm::BasicBlock::Create(llvm::getGlobalContext()));
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

void CodeGen::gen_code(NExpression* root) {
}

void CodeGen::run_code() {
}

llvm::Value* NPrimitiveNumber::gen_code(CodeGen* code_gen) {
	std::cout << "Generating int..." << std::endl;
	llvm::Function* number_create_fn = code_gen->module->getFunction("number_create");
	if (number_create_fn == NULL) {
		error("Unable to create number");
	}
	return builder.CreateCall(number_create_fn, std::vector<llvm::Value*>(), "calltmp");
	// return llvm_value_for_primitive_number(this);
}

llvm::Value* NBinaryOperator::gen_code(CodeGen* code_gen) {
	std::cout << "Generating binary operator..." << std::endl;
	llvm::Value* l = this->lhs->gen_code(code_gen);
	llvm::Value* r = this->rhs->gen_code(code_gen);
	if (l == NULL || r == NULL) {
		return NULL;
	}
	llvm::Function* number_binary_op_fn = NULL;
	builder.SetInsertPoint(code_gen->current_block());
	switch (this->op) {
		case eADD:
			// return builder.CreateAdd(l, r, "addtmp");
			number_binary_op_fn = code_gen->module->getFunction("number_add");
		case eSUBTRACT:
			// return builder.CreateSub(l, r, "subtmp");
			number_binary_op_fn = code_gen->module->getFunction("number_sub");
		case eMULTIPLY:
			// return builder.CreateMul(l, r, "multmp");
			number_binary_op_fn = code_gen->module->getFunction("number_mul");
		case eDIVIDE:
			// return builder.CreateSDiv(l, r, "divtmp");
			number_binary_op_fn = code_gen->module->getFunction("number_div");
		case ePOW:
			// return builder.CreateAdd(l, r, "powtmp");
			number_binary_op_fn = code_gen->module->getFunction("number_add");
	}
	if (number_binary_op_fn == NULL) {
		error("Unable to get number binary operator");
		return NULL;
	}
	std::vector<llvm::Value*> args;
	args.push_back(l);
	args.push_back(r);
	return builder.CreateCall(number_binary_op_fn, args, "calltmp");
}

llvm::Value* NFunction::gen_code(CodeGen* code_gen) {
	std::vector<llvm::Type*> arg_types;
	llvm::FunctionType* fn_type = llvm::FunctionType::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), arg_types, false);
	llvm::Function* fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, "", code_gen->module);
	
	llvm::BasicBlock* basic_block = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", fn);
	code_gen->push_block(basic_block);
	builder.SetInsertPoint(basic_block);

	llvm::Value* ret_val = this->body->gen_code(code_gen);
	if (ret_val != NULL) {
		builder.CreateRet(ret_val);
		llvm::verifyFunction(*fn);
		code_gen->pop_block();
		return fn;
	}
	code_gen->pop_block();
	error("Error generating function");
	return NULL;
}

llvm::Value* NIdentifier::gen_code(CodeGen* code_gen) {
	llvm::Value* val = code_gen->scope.get(this->name);
	if (val == NULL) {
		std::stringstream ss;
		ss << "Undeclared variable " << this->name;
		error(ss.str());
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
	std::cout << "Generating variable declaration for " << this->var_name->name << ", type " << this->type->name << "..." << std::endl;
	builder.SetInsertPoint(code_gen->current_block());
	// llvm::Type* declared_type = llvm_type_for_primitive_number(this->type->name);
	llvm::Type* declared_type = NULL;
	llvm::AllocaInst* alloc = new llvm::AllocaInst(declared_type, this->var_name->name.c_str(), code_gen->current_block()); //builder.CreateAlloca(llvm::Type::getInt64Ty(llvm::getGlobalContext()), 0, this->var_name->name.c_str());
	code_gen->scope.put(this->var_name->name, alloc);
	//return llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), 0, true);
	return alloc;
}

llvm::Value* NBlock::gen_code(CodeGen* code_gen) {
	llvm::Value* last = NULL;
	for (std::vector<NExpression*>::iterator it = this->statements.begin(); it != this->statements.end(); it++) {
		last = (*it)->gen_code(code_gen);
	}
	return last;
}

llvm::Type* CodeGen::llvm_pointer_ty() {
	static llvm::Type* ty = NULL;
	if (ty == NULL) {
		if (sizeof(void*) == 8) {
			ty = llvm::Type::getInt64Ty(llvm::getGlobalContext());
		} else if (sizeof(void*) == 4) {
			ty = llvm::Type::getInt32Ty(llvm::getGlobalContext());
		} else {
			std::stringstream ss;
			ss << "Unknown pointer size " << sizeof(void*);
			error(ss.str());
			return NULL;
		}
	}
	return ty;
}
