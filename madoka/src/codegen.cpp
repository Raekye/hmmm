#include "node.h"
#include "codegen.h"
#include "parser.h"
#include <iostream>
#include <boost/lexical_cast.hpp>
#include "number.h"
#include <sstream>

// TOOD: generalize/review type propagation

static void error(std::string str) {
	std::cout << str << std::endl;
}

CodeGen::CodeGen(llvm::Module* module) : builder(llvm::getGlobalContext()) {
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
	if (this->type->name == "double") {
		return llvm::ConstantFP::get(this->type->llvm_type, boost::lexical_cast<double>(this->str));
	} else if (this->type->name == "float") {
		return llvm::ConstantFP::get(this->type->llvm_type, boost::lexical_cast<float>(this->str));
	}
	if (this->type->is_primitive()) {
		if (this->type->is_signed()) {
			return llvm::ConstantInt::get(this->type->llvm_type, boost::lexical_cast<int64_t>(this->str), true);
		} else {
			return llvm::ConstantInt::get(this->type->llvm_type, boost::lexical_cast<uint64_t>(this->str), false);
		}
	}
	std::cout << "Unknown primitive number type " << this->type->name << std::endl;
	return NULL;
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
			return code_gen->builder.CreateAdd(l, r, "addtmp");
		case eSUBTRACT:
			return code_gen->builder.CreateSub(l, r, "subtmp");
		case eMULTIPLY:
			return code_gen->builder.CreateMul(l, r, "multmp");
		case eDIVIDE:
			return code_gen->builder.CreateSDiv(l, r, "divtmp");
		case ePOW:
			return code_gen->builder.CreateAdd(l, r, "powtmp"); // TODO: library function
	}
	std::cout << "Unknown binary operation" << std::endl;
	return NULL;
}

llvm::Value* NFunction::gen_code(CodeGen* code_gen) {
	std::cout << "Generating function..." << std::endl;
	std::vector<llvm::Type*> arg_types;
	llvm::FunctionType* fn_type = llvm::FunctionType::get(this->return_type->llvm_type, arg_types, false);
	llvm::Function* fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, "", code_gen->module);
	
	llvm::BasicBlock* basic_block = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", fn);
	code_gen->push_block(basic_block);
	code_gen->builder.SetInsertPoint(basic_block);

	llvm::Value* ret_val = this->body->gen_code(code_gen);
	if (ret_val != NULL) {
		// TODO: other casts
		llvm::Value* ret_val_casted = CodeGen::llvm_implicit_cast_primitive_number(code_gen, this->body->type, ret_val, this->return_type);
		if (ret_val_casted != NULL) {
			code_gen->builder.CreateRet(ret_val_casted);
			llvm::verifyFunction(*fn);
			code_gen->pop_block();
			return fn;
		}
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
	this->type = val->type;
	this->rhs->type = val->type;
	llvm::Value* rhs_val = this->rhs->gen_code(code_gen);
	code_gen->builder.SetInsertPoint(code_gen->current_block());
	code_gen->builder.CreateStore(rhs_val, val->value, false);
	return rhs_val;
}

llvm::Value* NVariableDeclaration::gen_code(CodeGen* code_gen) {
	if (this->type == NULL || this->type->llvm_type == NULL) {
		std::cout << "Unknown type " << this->type_name->name << std::endl;
		return NULL;
	}
	std::cout << "Generating variable declaration for " << this->var_name->name << ", type " << this->type->name << "..." << std::endl;
	code_gen->builder.SetInsertPoint(code_gen->current_block());
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
		this->type = (*it)->type;
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

llvm::Value* CodeGen::llvm_implicit_cast_primitive_number(CodeGen* code_gen, NType* source_type, llvm::Value* source_val, NType* dest_type) {
	if (!source_type->is_primitive() || !dest_type->is_primitive()) {
		throw std::logic_error("Can only implicit cast primitive number");
	}
	if (source_type == NType::double_ty()) {
		if (dest_type == NType::double_ty()) {
			return source_val;
		} else {
			return NULL;
		}
	} else if (source_type == NType::float_ty()) {
		if (dest_type == NType::float_ty()) {
			return source_val;
		} else if (dest_type == NType::double_ty()) {
			return code_gen->builder.CreateFPCast(source_val, dest_type->llvm_type);
		} else {
			return NULL;
		}
	}
	if (((llvm::IntegerType*) source_type->llvm_type)->getBitWidth() <= ((llvm::IntegerType*) dest_type->llvm_type)->getBitWidth()) {
		return CodeGen::llvm_cast_primitive_number(code_gen, source_type, source_val, dest_type);
	}
	return NULL;
}

llvm::Value* CodeGen::llvm_cast_primitive_number(CodeGen* code_gen, NType* source_type, llvm::Value* source_val, NType* dest_type) {
	if (!source_type->is_primitive() || !dest_type->is_primitive()) {
		throw std::logic_error("Can only cast primitive number");
	}
	if (source_type->is_floating()) {
		if (dest_type->is_floating()) {
			return code_gen->builder.CreateFPCast(source_val, dest_type->llvm_type);
		} else if (dest_type->is_signed()) {
			return code_gen->builder.CreateFPToSI(source_val, dest_type->llvm_type);
		} else {
			return code_gen->builder.CreateFPToUI(source_val, dest_type->llvm_type);
		}
	} else {
		if (dest_type->is_floating()) {
			if (source_type->is_signed()) {
				return code_gen->builder.CreateSIToFP(source_val, dest_type->llvm_type);
			} else {
				return code_gen->builder.CreateUIToFP(source_val, dest_type->llvm_type);
			}
		} else {
			return code_gen->builder.CreateIntCast(source_val, dest_type->llvm_type, dest_type->is_signed());
		}
	}
}
