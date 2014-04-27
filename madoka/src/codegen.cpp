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
	std::cout << "Generating primitve number " << this->type->name << "..." << std::endl;
	// TODO: cast
	return llvm::ConstantInt::get(this->type->llvm_type, this->val.l, this->type->name[0] != 'u');
	// return llvm::ConstantFP::get(this->type->llvm_type, this->val.d);
}

llvm::Value* NBinaryOperator::gen_code(CodeGen* code_gen) {
	std::cout << "Generating binary operator..." << std::endl;
	this->lhs->type = this->type;
	this->rhs->type = this->type;
	llvm::Value* l = this->lhs->gen_code(code_gen);
	llvm::Value* r = this->rhs->gen_code(code_gen);
	if (l == NULL || r == NULL) {
		return NULL;
	}
	switch (this->op) {
		case eADD:
			// return builder.CreateAdd(l, r, "addtmp");
		case eSUBTRACT:
			// return builder.CreateSub(l, r, "subtmp");
		case eMULTIPLY:
			// return builder.CreateMul(l, r, "multmp");
		case eDIVIDE:
			// return builder.CreateSDiv(l, r, "divtmp");
		case ePOW:
			// return builder.CreateAdd(l, r, "powtmp");
			do { } while (0);
	}
	return NULL;
}

llvm::Value* NFunction::gen_code(CodeGen* code_gen) {
	std::cout << "Generating function..." << std::endl;
	std::vector<llvm::Type*> arg_types;
	llvm::FunctionType* fn_type = llvm::FunctionType::get(this->return_type->llvm_type, arg_types, false);
	llvm::Function* fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, "", code_gen->module);
	
	llvm::BasicBlock* basic_block = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", fn);
	code_gen->push_block(basic_block);
	builder.SetInsertPoint(basic_block);

	llvm::Value* ret_val = this->body->gen_code(code_gen);
	if (ret_val != NULL) {
		// TODO: other casts
		llvm::Value* ret_val_casted = builder.CreateIntCast(ret_val, this->return_type->llvm_type, this->return_type->name[0] != 'u');
		builder.CreateRet(ret_val_casted);
		llvm::verifyFunction(*fn);
		code_gen->pop_block();
		return fn;
	}
	code_gen->pop_block();
	error("Error generating function");
	return NULL;
}

llvm::Value* NIdentifier::gen_code(CodeGen* code_gen) {
	std::cout << "Generating identifier " << this->name << std::endl;
	NExpression* val = code_gen->scope.get(this->name);
	if (val == NULL) {
		std::stringstream ss;
		ss << "Undeclared variable " << this->name;
		error(ss.str());
		return NULL;
	}
	return new llvm::LoadInst(val->value, "", false, code_gen->current_block());
}

llvm::Value* NAssignment::gen_code(CodeGen* code_gen) {
	std::cout << "Generating assignment to " << this->lhs->name << "..." << std::endl;
	NExpression* val = code_gen->scope.get(this->lhs->name);
	if (val == NULL) {
		std::cout << "Undeclared variable " << this->lhs->name << std::endl;
		return NULL;
	}
	// TODO: cast
	this->rhs->type = val->type;
	llvm::Value* rhs_val = this->rhs->gen_code(code_gen);
	builder.SetInsertPoint(code_gen->current_block());
	builder.CreateStore(rhs_val, val->value, false);
	return rhs_val;
}

llvm::Value* NVariableDeclaration::gen_code(CodeGen* code_gen) {
	if (this->type == NULL || this->type->llvm_type == NULL) {
		std::cout << "Unknown type " << this->type_name->name << std::endl;
		return NULL;
	}
	std::cout << "Generating variable declaration for " << this->var_name->name << ", type " << this->type->name << "..." << std::endl;
	builder.SetInsertPoint(code_gen->current_block());
	llvm::AllocaInst* alloc = new llvm::AllocaInst(this->type->llvm_type, this->var_name->name.c_str(), code_gen->current_block());
	this->value = alloc;
	code_gen->scope.put(this->var_name->name, this);
	return alloc;
}

llvm::Value* NBlock::gen_code(CodeGen* code_gen) {
	std::cout << "Generating block..." << std::endl;
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
