#include "ast_node.h"

#include <iostream>

ASTNodeFunction::ASTNodeFunction(ASTNodeBlock* body, std::string return_type) {
	this->body = body;
	this->return_type = return_type;
}

ASTNodeFunction::~ASTNodeFunction() {
	delete this->body;
}

ASTNodeFunction* ASTNodeFunction::pass_types(CodeGenContext* code_gen_context, ASTType* ignore) {
	this->type = NULL; // TODO;
	this->body = this->body->pass_types(code_gen_context, code_gen_context->ast_types_resolver.get(this->return_type));
	return this;
}

llvm::Value* ASTNodeFunction::gen_code(CodeGenContext* code_gen_context) {
	std::cout << "Generating function" << std::endl;
	std::vector<llvm::Type*> arg_types;
	ASTType* type = code_gen_context->ast_types_resolver.get(this->return_type);
	if (type == NULL) {
		throw std::runtime_error("Unknown type");
	}
	llvm::FunctionType* fn_type = llvm::FunctionType::get(type->llvm_type, arg_types, false);
	llvm::Function* fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, "", code_gen_context->module);
	
	llvm::BasicBlock* basic_block = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", fn);
	code_gen_context->push_block(basic_block);

	llvm::Value* ret_val = this->body->gen_code(code_gen_context);
	if (ret_val != NULL) {
		code_gen_context->builder.CreateRet(ret_val);
		llvm::verifyFunction(*fn);
		code_gen_context->pop_block();
		return fn;
	}
	code_gen_context->pop_block();
	throw std::runtime_error("Error generating function");
}
