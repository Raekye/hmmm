#include "ast_node_function.h"

#include <iostream>

ASTNodeFunction::ASTNodeFunction(ASTNode* body, ASTType* return_type) {
	this->body = body;
	this->return_type = return_type;
}

ASTNodeFunction::~ASTNodeFunction() {
	delete this->body;
}

llvm::Value* ASTNodeFunction::gen_code(CodeGen* code_gen) {
	std::cout << "Generating function..." << std::endl;
	std::vector<llvm::Type*> arg_types;
	llvm::FunctionType* fn_type = llvm::FunctionType::get(this->return_type->llvm_type, arg_types, false);
	llvm::Function* fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, "", code_gen->module);
	
	llvm::BasicBlock* basic_block = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", fn);
	code_gen->push_block(basic_block);
	code_gen->builder.SetInsertPoint(basic_block);

	llvm::Value* ret_val = this->body->gen_code(code_gen);
	if (ret_val != NULL) {
		code_gen->builder.CreateRet(ret_val);
		llvm::verifyFunction(*fn);
		code_gen->pop_block();
		return fn;
	}
	code_gen->pop_block();
	throw std::runtime_error("Error generating function");
}
