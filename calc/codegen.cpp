#include "node.h"
#include "codegen.h"
#include "parser.hpp"
#include <iostream>
#include <boost/lexical_cast.hpp>

static void Error(const char* str) {
	std::cout << str << std::endl;
}

static llvm::Value* ErrorV(const char* str) {
	Error(str);
	return NULL;
}

static llvm::IRBuilder<> builder(llvm::getGlobalContext());

// static llvm::Value* llvm_value_for_primitive_number(std::string type, NPrimitiveNumber* num) {
// 	if (type == "byte") {
// 		return llvm::ConstantInt::get(llvm::Type::getInt8Ty(llvm::getGlobalContext()), boost::lexical_cast<int8_t>(num->str), true);
// 	} else if (type == "ubyte") {
// 		return llvm::ConstantInt::get(llvm::Type::getInt8Ty(llvm::getGlobalContext()), boost::lexical_cast<uint8_t>(num->str), false);
// 	} else if (type == "short") {
// 		return llvm::ConstantInt::get(llvm::Type::getInt16Ty(llvm::getGlobalContext()), boost::lexical_cast<int16_t>(num->str), true);
// 	} else if (type == "ushort") {
// 		return llvm::ConstantInt::get(llvm::Type::getInt16Ty(llvm::getGlobalContext()), boost::lexical_cast<uint16_t>(num->str), false);
// 	} else if (type == "int") {
// 		return llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvm::getGlobalContext()), boost::lexical_cast<int32_t>(num->str), true);
// 	} else if (type == "uint") {
// 		return llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvm::getGlobalContext()), boost::lexical_cast<uint32_t>(num->str), false);
// 	} else if (type == "long") {
// 		return llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), boost::lexical_cast<int64_t>(num->str), true);
// 	} else if (type == "ulong") {
// 		return llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), boost::lexical_cast<uint64_t>(num->str), false);
// 	} if (type == "float") {
// 		return llvm::ConstantFP::get(llvm::Type::getFloatTy(llvm::getGlobalContext()), boost::lexical_cast<float>(num->str));
// 	} else if (type == "double") {
// 		return llvm::ConstantFP::get(llvm::Type::getDoubleTy(llvm::getGlobalContext()), boost::lexical_cast<double>(num->str));
// 	}
// 	return NULL;
// }

static llvm::Value* llvm_value_for_primitive_number(NPrimitiveNumber* num) {
	if (num->type == eBYTE) {
		return llvm::ConstantInt::get(llvm::Type::getInt8Ty(llvm::getGlobalContext()), num->val.b, true);
	} else if (num->type == eUBYTE) {
		return llvm::ConstantInt::get(llvm::Type::getInt8Ty(llvm::getGlobalContext()), num->val.ub, false);
	} else if (num->type == eSHORT) {
		return llvm::ConstantInt::get(llvm::Type::getInt16Ty(llvm::getGlobalContext()), num->val.s, true);
	} else if (num->type == eUSHORT) {
		return llvm::ConstantInt::get(llvm::Type::getInt16Ty(llvm::getGlobalContext()), num->val.us, false);
	} else if (num->type == eINT) {
		return llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvm::getGlobalContext()), num->val.i, true);
	} else if (num->type == eUINT) {
		return llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvm::getGlobalContext()), num->val.ui, false);
	} else if (num->type == eLONG) {
		return llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), num->val.l, true);
	} else if (num->type == eULONG) {
		return llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvm::getGlobalContext()), num->val.ul, false);
	} else if (num->type == eFLOAT) {
		return llvm::ConstantFP::get(llvm::Type::getFloatTy(llvm::getGlobalContext()), num->val.f);
	} else if (num->type == eDOUBLE) {
		return llvm::ConstantFP::get(llvm::Type::getDoubleTy(llvm::getGlobalContext()), num->val.d);
	}
	return NULL;
}

static llvm::Type* llvm_type_for_primitive_number(std::string type) {
	if (type == "byte") {
		return llvm::Type::getInt8Ty(llvm::getGlobalContext());
	} else if (type == "ubyte") {
		return llvm::Type::getInt8Ty(llvm::getGlobalContext());
	} else if (type == "short") {
		return llvm::Type::getInt16Ty(llvm::getGlobalContext());
	} else if (type == "ushort") {
		return llvm::Type::getInt16Ty(llvm::getGlobalContext());
	} else if (type == "int") {
		return llvm::Type::getInt32Ty(llvm::getGlobalContext());
	} else if (type == "uint") {
		return llvm::Type::getInt32Ty(llvm::getGlobalContext());
	} else if (type == "long") {
		return llvm::Type::getInt64Ty(llvm::getGlobalContext());
	} else if (type == "ulong") {
		return llvm::Type::getInt64Ty(llvm::getGlobalContext());
	} if (type == "float") {
		return llvm::Type::getFloatTy(llvm::getGlobalContext());
	} else if (type == "double") {
		return llvm::Type::getDoubleTy(llvm::getGlobalContext());
	}
	return NULL;
}

static ENumberType enumbertype_from_llvm_type(llvm::Type* t) {
	if (t == llvm_type_for_primitive_number("byte")) {
		return eBYTE;
	} else if (t == llvm_type_for_primitive_number("ubyte")) {
		return eUBYTE;
	} else if (t == llvm_type_for_primitive_number("short")) {
		return eSHORT;
	} else if (t == llvm_type_for_primitive_number("ushort")) {
		return eUSHORT;
	} else if (t == llvm_type_for_primitive_number("int")) {
		return eINT;
	} else if (t == llvm_type_for_primitive_number("uint")) {
		return eUINT;
	} else if (t == llvm_type_for_primitive_number("long")) {
		return eLONG;
	} else if (t == llvm_type_for_primitive_number("ulong")) {
		return eULONG;
	} else if (t == llvm_type_for_primitive_number("float")) {
		return eFLOAT;
	} else if (t == llvm_type_for_primitive_number("double")) {
		return eDOUBLE;
	}
	// throw std::invalid_argument("Unknown type.");
	return eBYTE;
}

static bool llvm_type_can_assign_to(ENumberType source, ENumberType target) {
	if (target == eDOUBLE) {
		return true;
	}
	if (source > target) {
		return false;
	}
	return true;
}

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
	return llvm_value_for_primitive_number(this);
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
	llvm::FunctionType* fn_type = llvm::FunctionType::get(llvm::Type::getInt8Ty(llvm::getGlobalContext()), arg_types, false);
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
	std::cout << "Generating variable declaration for " << this->var_name->name << ", type " << this->type->name << "..." << std::endl;
	builder.SetInsertPoint(code_gen->current_block());
	llvm::Type* declared_type = llvm_type_for_primitive_number(this->type->name);
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